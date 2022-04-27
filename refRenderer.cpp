#include <algorithm>
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <vector>

#include "image.h"
#include "refRenderer.h"
#include "util.h"

RefRenderer::RefRenderer() {
    image = NULL;

    numberOfTrees = 0;
    position = NULL;
    color = NULL;
}

RefRenderer::~RefRenderer() {
    if (image) {
        delete image;
    }

    if (position) {
        delete[] position;
        delete[] color;
    }
}

const Image *RefRenderer::getImage() { return image; }

void RefRenderer::setup() {
    // nothing to do here
}

// allocOutputImage --
//
// Allocate buffer the renderer will render into.  Check status of
// image first to avoid memory leak.
void RefRenderer::allocOutputImage(int width, int height) {
    if (image)
        delete image;
    image = new Image(width, height);
}

// clearImage --
//
// Clear's the renderer's target image.  The state of the image after
// the clear depends on the scene being rendered.
void RefRenderer::clearImage() {
    image->clear(1.f, 1.f, 1.f, 1.f);
}

void RefRenderer::loadTrees(float* position, float* color, int numberOfTrees) {
    this->position = position;
    this->color = color;
    this->numberOfTrees = numberOfTrees;
}

// advanceAnimation --
//
// Advance the simulation one time step.  Updates all circle positions
// and velocities
void RefRenderer::advanceAnimation() {
    //Currently does NOTHING
}

static inline void lookupColor(float coord, float &r, float &g, float &b) {
    const int N = 5;

    float lookupTable[N][3] = {
        {1.f, 1.f, 1.f}, {1.f, 1.f, 1.f}, {.8f, .9f, 1.f}, {.8f, .9f, 1.f}, {.8f, 0.8f, 1.f},
    };

    float scaledCoord = coord * (N - 1);

    int base = std::min(static_cast<int>(scaledCoord), N - 1);

    // linearly interpolate between values in the table based on the
    // value of coord
    float weight = scaledCoord - static_cast<float>(base);
    float oneMinusWeight = 1.f - weight;

    r = (oneMinusWeight * lookupTable[base][0]) + (weight * lookupTable[base + 1][0]);
    g = (oneMinusWeight * lookupTable[base][1]) + (weight * lookupTable[base + 1][1]);
    b = (oneMinusWeight * lookupTable[base][2]) + (weight * lookupTable[base + 1][2]);
}

// shadePixel --
//
// Computes the contribution of the specified circle to the
// given pixel.  All values are provided in normalized space, where
// the screen spans [0,2]^2.  The color/opacity of the circle is
// computed at the pixel center.
void RefRenderer::shadePixel(float pixelCenterX, float pixelCenterY, float px, float py, float pz, float *pixelData,
                             int treeIndex) {
    float diffX = px - pixelCenterX;
    float diffY = py - pixelCenterY;
    float pixelDist = diffX * diffX + diffY * diffY;

    float rad = 0;
    float maxDist = rad * rad;

    // circle does not contribute to the image
    if (pixelDist > maxDist)
        return;

    float colR, colG, colB;
    float alpha;

    // simple: each circle has an assigned color
    int index3 = 3 * treeIndex;
    colR = color[index3];
    colG = color[index3 + 1];
    colB = color[index3 + 2];
    alpha = .5f;

    // The following code is *very important*: it blends the
    // contribution of the circle primitive with the current state
    // of the output image pixel.  This is a read-modify-write
    // operation on the image, and it needs to be atomic.  Moreover,
    // (and even more challenging) all writes to this pixel must be
    // performed in same order as when the circles are processed
    // serially.
    //
    // That is, if circle 1 and circle 2 both write to pixel P.
    // circle 1's contribution *must* be blended in first, then
    // circle 2's.  If this invariant is not preserved, the
    // rendering of transparent circles will not be correct.

    float oneMinusAlpha = 1.f - alpha;
    pixelData[0] = alpha * colR + oneMinusAlpha * pixelData[0];
    pixelData[1] = alpha * colG + oneMinusAlpha * pixelData[1];
    pixelData[2] = alpha * colB + oneMinusAlpha * pixelData[2];
    pixelData[3] += alpha;
}

void RefRenderer::render() {
    // Render all circles
    for (int treeIndex = 0; treeIndex < numberOfTrees; treeIndex++) {

        int index3 = 3 * treeIndex;

        float px = position[index3];
        float py = position[index3 + 1];
        float pz = position[index3 + 2];

        float colR, colG, colB;
        colR = color[index3];
        colG = color[index3 + 1];
        colB = color[index3 + 2];


        // Convert normalized coordinates to integer screen
        // pixels.  Clamp to the edges of the screen.
        int pixelX = CLAMP(static_cast<int>(px * image->width), 0, image->width);
        int pixelY = CLAMP(static_cast<int>(py * image->height), 0, image->height);

        float invWidth = 1.f / image->width;
        float invHeight = 1.f / image->height;

        // For each pixel in the bounding box, determine the circle's
        // contribution to the pixel.  The contribution is computed in
        // the function shadePixel.  Since the circle does not fill
        // the bounding box entirely, not every pixel in the box will
        // receive contribution.

        float *imgPtr = &image->data[4 * (pixelY * image->width + pixelX)];

        imgPtr[0] = colR;
        imgPtr[1] = colG;
        imgPtr[2] = colB;
        imgPtr[3] = 1;

        continue;
    }

    return;


    // UNUSED
    /*
    for (int pixelY = screenMinY; pixelY < screenMaxY; pixelY++) {

        // pointer to pixel data
        float *imgPtr = &image->data[4 * (pixelY * image->width + screenMinX)];

        for (int pixelX = screenMinX; pixelX < screenMaxX; pixelX++) {

            // When "shading" the pixel ("shading" = computing the
            // circle's color and opacity at the pixel), we treat
            // the pixel as a point at the center of the pixel.
            // We'll compute the color of the circle at this
            // point.  Note that shading math will occur in the
            // normalized [0,1]^2 coordinate space, so we convert
            // the pixel center into this coordinate space prior
            // to calling shadePixel.
            float pixelCenterNormX = invWidth * (static_cast<float>(pixelX) + 0.5f);
            float pixelCenterNormY = invHeight * (static_cast<float>(pixelY) + 0.5f);
            shadePixel(pixelCenterNormX, pixelCenterNormY, px, py, pz, imgPtr, treeIndex);
            imgPtr += 4;
        }
    }
    */
}

/*
void RefRenderer::dumpParticles(const char *filename) {

    FILE *output = fopen(filename, "w");

    fprintf(output, "%d\n", numberOfTrees);
    for (int i = 0; i < numberOfTrees; i++) {
        fprintf(output, "%f %f %f   %f %f %f   %f\n",
            position[3 * i + 0], position[3 * i + 1], position[3 * i + 2],
            velocity[3 * i + 0], velocity[3 * i + 1], velocity[3 * i + 2],
            radius[i]);
    }
    fclose(output);
}
*/
