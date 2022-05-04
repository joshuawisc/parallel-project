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

////////////////////////////////////////////////////////////////////////////////////////
// All cuda kernels here
///////////////////////////////////////////////////////////////////////////////////////

// This stores the global constants
struct GlobalConstants {

    int numberOfTrees;

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


////////////////////////////////////////////////////////////////////////////////////////

CudaRenderer::CudaRenderer() {
    image = NULL;

    numberOfTrees = 0;
    position = NULL;
    color = NULL;

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

    printf("Copying image data from device\n");

    cudaMemcpy(image->data, cudaDeviceImageData, sizeof(float) * 4 * image->width * image->height,
               cudaMemcpyDeviceToHost);

    return image;
}

void CudaRenderer::loadTrees(LSystem *trees, int numberOfTrees) {
    this->trees = trees;
    this->numberOfTrees = numberOfTrees;
}

void CudaRenderer::setup() {
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
        if (name.compare("GeForce RTX 2080") == 0) {
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
    int numberOfLines = trees[0].numLines();
    cudaMalloc(&cudaDevicePosition, sizeof(float) * 4 * numberOfTrees * numberOfLines);
    cudaMalloc(&cudaDeviceColor, sizeof(float) * 3 * numberOfTrees);
    cudaMalloc(&cudaDeviceImageData, sizeof(float) * 4 * image->width * image->height);

    for (int i = 0 ; i < numberOfTrees ; i++) {
        cudaMemcpy(cudaDevicePosition + sizeof(float)*4*numberOfLines*i, trees[i].lines, sizeof(float) * 4 * numberOfLines, cudaMemcpyHostToDevice);
        cudaMemcpy(cudaDeviceColor + sizeof(float)*3*i, trees[i].color, sizeof(float) * 3, cudaMemcpyHostToDevice);
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
    params.imageWidth = image->width;
    params.imageHeight = image->height;
    params.position = cudaDevicePosition;
    params.color = cudaDeviceColor;
    params.imageData = cudaDeviceImageData;

    cudaMemcpyToSymbol(cuConstRendererParams, &params, sizeof(GlobalConstants));

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

    // 256 threads per block is a healthy number
    dim3 blockDim(16, 16, 1);
    dim3 gridDim((image->width + blockDim.x - 1) / blockDim.x,
                 (image->height + blockDim.y - 1) / blockDim.y);

    kernelClearImage<<<gridDim, blockDim>>>(1.f, 1.f, 1.f, 1.f);
    cudaDeviceSynchronize();
}

// Does nothing
void CudaRenderer::advanceAnimation() {
    return;
}

void CudaRenderer::render() {
    // 256 threads per block is a healthy number
    dim3 blockDim(256, 1);
    dim3 gridDim((numberOfTrees + blockDim.x - 1) / blockDim.x);

    // TODO: Render lines
    // kernelRenderCircles<<<gridDim, blockDim>>>();
    cudaDeviceSynchronize();
}
