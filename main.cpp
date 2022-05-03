#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "platformgl.h"
#include "refRenderer.h"
#include "cycleTimer.h"

// randomFloat --
// //
// // return a random floating point value between 0 and 1
static float randomFloat() { return static_cast<float>(rand()) / RAND_MAX; }


void startRendererWithDisplay(RefRenderer *renderer);
/*
void startBenchmark(RefRenderer *renderer, int startFrame, int totalFrames,
                    const std::string &frameFilename);
void checkBenchmark(RefRenderer *ref_renderer, RefRenderer *cuda_renderer,
                    int benchmarkFrameStart, int totalFrames,
                    const std::string &frameFilename);
*/

/*
void usage(const char *progname) {
    printf("Usage: %s [options] scenename\n", progname);
    printf("Valid scenenames are: rgb, rgby, rand10k, rand100k, biglittle, littlebig, "
           "pattern, bouncingballs, fireworks, hypnosis, snow, snowsingle\n");
    printf("Program Options:\n");
    printf("  -b  --bench <START:END>    Benchmark mode, do not create display. Time frames [START,END)\n");
    printf("  -c  --check                Check correctness of output\n");
    printf("  -f  --file  <FILENAME>     Dump frames in benchmark mode (FILENAME_xxxx.ppm)\n");
    printf("  -r  --renderer <ref/cuda>  Select renderer: ref or cuda\n");
    printf("  -s  --size  <INT>          Make rendered image <INT>x<INT> pixels\n");
    printf("  -?  --help                 This message\n");
}
*/

int main(int argc, char **argv) {
    int imageSize = 1150;

    std::string frameFilename;

    printf("Rendering to %dx%d image\n", imageSize, imageSize);

    RefRenderer *renderer;
    
    int numberOfTrees = 20000;
    LSystem trees[numberOfTrees];

    int threads = 8;

    double preGenerate = CycleTimer::currentSeconds();
    int depth = 5;
    for (int i = 0; i < numberOfTrees ; i+=2) {
        float length = .3;

        float x = randomFloat(), y = randomFloat(); // initial position
        float angle = 3.14/2; // initial angle
        float colors[3] = {randomFloat()*0. + 1, randomFloat()*1.0 + 0, randomFloat()*0.5 + 0};
        Fern L(x, y, angle, length/(1<<depth), colors);
        L.generate(depth);
        trees[i] = L;
    }
    double endGenerate = CycleTimer::currentSeconds();

    printf("Create %d trees with depth %d in %.3f ms\n", numberOfTrees, depth, 1000.f * (endGenerate - preGenerate));

    renderer = new RefRenderer();

    renderer->allocOutputImage(imageSize, imageSize);
    renderer->loadTrees(trees, numberOfTrees);
    renderer->setup();

    glutInit(&argc, argv);
    startRendererWithDisplay(renderer);

    return 0;
}
