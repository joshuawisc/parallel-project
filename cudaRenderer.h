#ifndef __CUDA_RENDERER_H__
#define __CUDA_RENDERER_H__

#ifndef uint
#define uint unsigned int
#endif

#include "treeRenderer.h"

class CudaRenderer : public TreeRenderer {

  private:
    Image *image;

    int numberOfTrees;
    int numberOfLines;
    float *position;
    float *color;
    LSystem *trees;

    float *cudaDevicePosition;
    float *cudaDeviceColor;
    float *cudaDeviceImageData;

  public:
    CudaRenderer();
    virtual ~CudaRenderer();

    const Image *getImage();

    void setup(int threads);

    void loadTrees(LSystem *trees, int numberOfTrees);

    void allocOutputImage(int width, int height);

    void clearImage();

    void advanceAnimation();

    void render();

    void shadePixel(float pixelCenterX, float pixelCenterY, float px, float py, float pz,
                    float *pixelData, int circleIndex);
};

#endif
