#ifndef __TREE_RENDERER_H__
#define __TREE_RENDERER_H__

#include "lsystem.cpp"

struct Image;

class TreeRenderer {
  public:
    virtual ~TreeRenderer(){};

    virtual const Image *getImage() = 0;

    virtual void setup(int threads) = 0;

    virtual void loadTrees(LSystem *trees, int numberOfTrees) = 0;

    virtual void allocOutputImage(int width, int height) = 0;

    virtual void clearImage() = 0;

    virtual void advanceAnimation() = 0;

    virtual void render() = 0;

    // virtual void dumpParticles(const char* filename) {}
};

#endif
