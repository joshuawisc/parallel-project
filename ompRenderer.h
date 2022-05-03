#ifndef __OMP_RENDERER_H__
#define __OMP_RENDERER_H__

#include "treeRenderer.h"

class OmpRenderer : public TreeRenderer{
  private:
    Image *image;

    int numberOfTrees;
    float *position;
    float *color;
    LSystem *trees;

  public:
    OmpRenderer();
    virtual ~OmpRenderer();

    const Image *getImage();

    void setup(int threads);

    void loadTrees(LSystem *trees, int numberOfTrees);

    void allocOutputImage(int width, int height);

    void clearImage();

    void advanceAnimation();

    void render();

    void drawLine(float x0, float y0, float x1, float y1, LSystem ls);

    void drawTree(LSystem ls);



    //void dumpParticles(const char *filename);

    void shadePixel(float pixelCenterX, float pixelCenterY, float px, float py, float pz,
                    float *pixelData, int treeIndex);
};

#endif
