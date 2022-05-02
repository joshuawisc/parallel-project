#ifndef __REF_RENDERER_H__
#define __REF_RENDERER_H__

#include "generation_class.cpp"


struct Image;

class RefRenderer {
  private:
    Image *image;

    int numberOfTrees;
    float *position;
    float *color;
    LSystem *trees;

  public:
    RefRenderer();
    virtual ~RefRenderer();

    const Image *getImage();

    void setup();

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
