#ifndef __REF_RENDERER_H__
#define __REF_RENDERER_H__

struct Image;

class RefRenderer {
  private:
    Image *image;

    int numberOfTrees;
    float *position;
    float *color;

  public:
    RefRenderer();
    virtual ~RefRenderer();

    const Image *getImage();

    void setup();

    void loadTrees(float* position, float* color, int numberOfTrees);

    void allocOutputImage(int width, int height);

    void clearImage();

    void advanceAnimation();

    void render();

    //void dumpParticles(const char *filename);

    void shadePixel(float pixelCenterX, float pixelCenterY, float px, float py, float pz,
                    float *pixelData, int treeIndex);
};

#endif
