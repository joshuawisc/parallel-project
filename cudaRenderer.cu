#include <algorithm>
#include <string>
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <vector>

#include <cuda.h>
#include <cuda_runtime.h>
#include <driver_functions.h>

#include "cudaRenderer.h"
#include "util.h"
#include "image.h"
#include "cycleTimer.h"

////////////////////////////////////////////////////////////////////////////////////////
// All cuda kernels here
///////////////////////////////////////////////////////////////////////////////////////

// This stores the global constants
struct GlobalConstants {

    int numberOfTrees;
    int numberOfLines;

    float *position;
    float *color;
    LSystem *trees;

    int imageWidth;
    int imageHeight;
    float *imageData;
};

// Global variable that is in scope, but read-only, for all CUDA
// kernels.  The __constant__ modifier designates this variable will
// be stored in special "constant" memory on the GPU. (We didn't talk
// about this type of memory in class, but constant memory is a fast
// place to put read-only variables).
__constant__ GlobalConstants cuConstRendererParams;



// kernelClearImage --  (CUDA device code)
//
// Clear the image, setting all pixels to the specified color rgba
__global__ void kernelClearImage(float r, float g, float b, float a) {
    int imageX = blockIdx.x * blockDim.x + threadIdx.x;
    int imageY = blockIdx.y * blockDim.y + threadIdx.y;

    int width = cuConstRendererParams.imageWidth;
    int height = cuConstRendererParams.imageHeight;

    if (imageX >= width || imageY >= height)
        return;

    int offset = 4 * (imageY * width + imageX);
    float4 value = make_float4(r, g, b, a);

    // Write to global memory: As an optimization, this code uses a float4
    // store, which results in more efficient code than if it were coded as
    // four separate float stores.
    *(float4 *)(&cuConstRendererParams.imageData[offset]) = value;
}

__device__ void drawLine(float x0, float y0, float x1, float y1, float r, float g, float b) {
    // Can't do (1.0)??
    int width = cuConstRendererParams.imageWidth;
    int height = cuConstRendererParams.imageHeight;

    x1 = min(max(static_cast<int>(x1 * width), 0), width-1);
    y1 = min(max(static_cast<int>(y1 * height), 0), height-1);
    x0 = min(max(static_cast<int>(x0 * width), 0), width-1);
    y0 = min(max(static_cast<int>(y0 * height), 0), height-1);
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int error = dx + dy;
    int e2;

    while (true) {
        // plot(x0, y0);
        // TODO: USE FLOAT4 INSTEAD
        float *imgPtr = &cuConstRendererParams.imageData[4 * (int(y0) * width + int(x0))];
        imgPtr[0] = r;
        imgPtr[1] = g;
        imgPtr[2] = b;
        imgPtr[3] = 1;

        if (x0 == x1 && y0 == y1)
            break;
        e2 = 2 * error;
        if (e2 >= dy) {
            if (x0 == x1)
                break;
            error = error + dy;
            x0 = x0 + sx;
        }
        if (e2 <= dx) {
            if (y0 == y1)
                break;
            error = error + dx;
            y0 = y0 + sy;
        }
    }
}

__global__ void kernelRenderLines() {
    GlobalConstants params = cuConstRendererParams;
    int numberOfLines = params.numberOfLines;
    int numberOfTrees = params.numberOfTrees;
    int lineIndex = blockIdx.x * blockDim.x + threadIdx.x;
    int treeIndex = lineIndex / numberOfLines;
    if (lineIndex >= numberOfLines*numberOfTrees)
        return;
    lineIndex *= 4;
    treeIndex *= 3;
    drawLine(params.position[lineIndex], params.position[lineIndex+1], params.position[lineIndex+2], params.position[lineIndex+3],
        params.color[treeIndex], params.color[treeIndex+1], params.color[treeIndex+2]);

}


////////////////////////////////////////////////////////////////////////////////////////

CudaRenderer::CudaRenderer() {
    image = NULL;

    numberOfTrees = 0;
    numberOfLines = 0;
    position = NULL;
    color = NULL;
    trees = NULL;

    cudaDevicePosition = NULL;
    cudaDeviceColor = NULL;
    cudaDeviceImageData = NULL;
}

CudaRenderer::~CudaRenderer() {
    if (image) {
        delete image;
    }

    if (position) {
        delete[] position;
        delete[] color;
    }

    if (cudaDevicePosition) {
        cudaFree(cudaDevicePosition);
        cudaFree(cudaDeviceColor);
        cudaFree(cudaDeviceImageData);
    }
}

const Image *CudaRenderer::getImage() {
    // Need to copy contents of the rendered image from device memory
    // before we expose the Image object to the caller

    // printf("Copying image data from device\n");
    
    double preSetup = CycleTimer::currentSeconds();
    cudaMemcpy(image->data, cudaDeviceImageData, sizeof(float) * 4 * image->width * image->height,
               cudaMemcpyDeviceToHost);

    double postSetup = CycleTimer::currentSeconds();
    // printf("CUDA getImage time:\t%.3f ms\n", 1000.f * (postSetup - preSetup));
    return image;
}

void CudaRenderer::loadTrees(LSystem *trees, int numberOfTrees) {
    this->trees = trees;
    this->numberOfTrees = numberOfTrees;
}

void CudaRenderer::setup(int threads) {
    double preSetup = CycleTimer::currentSeconds();
    int deviceCount = 0;
    bool isFastGPU = false;
    std::string name;
    cudaError_t err = cudaGetDeviceCount(&deviceCount);

    printf("---------------------------------------------------------\n");
    printf("Initializing CUDA for CudaRenderer\n");
    printf("Found %d CUDA devices\n", deviceCount);

    for (int i = 0; i < deviceCount; i++) {
        cudaDeviceProp deviceProps;
        cudaGetDeviceProperties(&deviceProps, i);
        name = deviceProps.name;
        if (name.compare("NVIDIA GeForce RTX 2080") == 0) {
            isFastGPU = true;
        }

        printf("Device %d: %s\n", i, deviceProps.name);
        printf("   SMs:        %d\n", deviceProps.multiProcessorCount);
        printf("   Global mem: %.0f MB\n", static_cast<float>(deviceProps.totalGlobalMem) / (1024 * 1024));
        printf("   CUDA Cap:   %d.%d\n", deviceProps.major, deviceProps.minor);
    }
    printf("---------------------------------------------------------\n");
    if (!isFastGPU) {
        printf("WARNING: "
               "You're not running on a fast GPU, please consider using "
               "NVIDIA RTX 2080.\n");
        printf("---------------------------------------------------------\n");
    }

    // By this time the scene should be loaded.  Now copy all the key
    // data structures into device memory so they are accessible to
    // CUDA kernels
    //
    // See the CUDA Programmer's Guide for descriptions of
    // cudaMalloc and cudaMemcpy

    //TODO: Copy lines over correctly
    this->numberOfLines = trees[0].numLines(trees[0].depth);
    cudaMalloc(&cudaDevicePosition, sizeof(float) * 4 * numberOfTrees * numberOfLines);
    cudaMalloc(&cudaDeviceColor, sizeof(float) * 3 * numberOfTrees);
    cudaMalloc(&cudaDeviceImageData, sizeof(float) * 4 * image->width * image->height);

    for (int i = 0 ; i < numberOfTrees ; i++) {
        cudaMemcpy(cudaDevicePosition + 4*numberOfLines*i, &trees[i].lines.front(), sizeof(float) * 4 * numberOfLines, cudaMemcpyHostToDevice);
        cudaMemcpy(cudaDeviceColor + 3*i, trees[i].color, sizeof(float) * 3, cudaMemcpyHostToDevice);
    }
    // Initialize parameters in constant memory.  We didn't talk about
    // constant memory in class, but the use of read-only constant
    // memory here is an optimization over just sticking these values
    // in device global memory.  NVIDIA GPUs have a few special tricks
    // for optimizing access to constant memory.  Using global memory
    // here would have worked just as well.  See the Programmer's
    // Guide for more information about constant memory.

    GlobalConstants params;
    params.numberOfTrees = numberOfTrees;
    params.numberOfLines = numberOfLines;
    params.imageWidth = image->width;
    params.imageHeight = image->height;
    params.position = cudaDevicePosition;
    params.color = cudaDeviceColor;
    params.imageData = cudaDeviceImageData;
    params.trees = trees;

    cudaMemcpyToSymbol(cuConstRendererParams, &params, sizeof(GlobalConstants));
    double postSetup = CycleTimer::currentSeconds();
    printf("CUDA setup time:\t%.3f ms\n", 1000.f * (postSetup - preSetup));

}

// Allocate buffer the renderer will render into.  Check status of
// image first to avoid memory leak.
void CudaRenderer::allocOutputImage(int width, int height) {
    if (image)
        delete image;
    image = new Image(width, height);
}

// Clear the renderer's target image.  The state of the image after
// the clear depends on the scene being rendered.
void CudaRenderer::clearImage() {
    double preClear = CycleTimer::currentSeconds();
    // 256 threads per block is a healthy number
    dim3 blockDim(16, 16, 1);
    dim3 gridDim((image->width + blockDim.x - 1) / blockDim.x,
                 (image->height + blockDim.y - 1) / blockDim.y);

    kernelClearImage<<<gridDim, blockDim>>>(0.f, 0.f, 0.f, 1.f);
    cudaDeviceSynchronize();
    double postClear = CycleTimer::currentSeconds();
    printf("CUDA clear time:\t%.3f ms\n", 1000.f * (postClear - preClear));
}

// Does nothing
void CudaRenderer::advanceAnimation() {
    return;
}

void CudaRenderer::render() {
    // 256 threads per block is a healthy number
    dim3 blockDim(1024, 1);
    dim3 gridDim((numberOfTrees*numberOfLines + blockDim.x - 1) / blockDim.x);
    
    double preRender = CycleTimer::currentSeconds();
    // TODO: Render lines
    kernelRenderLines<<<gridDim, blockDim>>>();
    cudaDeviceSynchronize();
    double postRender = CycleTimer::currentSeconds();
    printf("CUDA render time:\t%.3f ms\n", 1000.f * (postRender - preRender));
}
