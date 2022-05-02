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
    trees = NULL;
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

void RefRenderer::loadTrees(LSystem *trees, int numberOfTrees) {
    this->trees = trees;
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

void RefRenderer::drawLine(float x0, float y0, float x1, float y1, LSystem ls) {
    // Can't do (1.0)??
    x1 = CLAMP(static_cast<int>(x1 * this->image->width)-1, 0, this->image->width-1);
    y1 = CLAMP(static_cast<int>(y1 * this->image->height)-1, 0, this->image->height-1);
    x0 = CLAMP(static_cast<int>(x0 * this->image->width)-1, 0, this->image->width-1);
    y0 = CLAMP(static_cast<int>(y0 * this->image->height)-1, 0, this->image->height-1);
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int error = dx + dy;
    int e2;

    while (true) {
        // plot(x0, y0);
        float *imgPtr = &image->data[4 * (int(y0) * image->width + int(x0))];
        imgPtr[0] = ls.color[0];
        imgPtr[1] = ls.color[1];
        imgPtr[2] = ls.color[2];
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

void RefRenderer::drawTree(LSystem ls) {

    // draw loop
    float angle = ls.angle;
    float x = ls.x;
    float y = ls.y;
    for (char c : ls.instructions) {
        if (c == 'F') {
            float new_x = x + ls.length * cos(angle);
            float new_y = y + ls.length * sin(angle);
            drawLine(x, y, new_x, new_y, ls);
            x = new_x, y = new_y;
        } else if (c == '+') {
            angle += ls.rotation;
        } else if (c == '-') {
            angle -= ls.rotation;
        } else if (c == '[') {
            // TODO: save current position and angle
        } else if (c == ']') {
            // TODO: restore current position and angle
        }
    }
}



void RefRenderer::render() {
    // Render all circles
    for (int treeIndex = 0; treeIndex < numberOfTrees; treeIndex++) {

        drawTree(trees[treeIndex]);
        /**
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
        **/
    }

    return;
}
