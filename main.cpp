#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "platformgl.h"
#include "ompRenderer.h"
#include "cycleTimer.h"

// randomFloat --
// //
// // return a random floating point value between 0 and 1
static float randomFloat() { return static_cast<float>(rand()) / RAND_MAX; }


void startRendererWithDisplay(TreeRenderer *renderer);
/*
void startBenchmark(OmpRenderer *renderer, int startFrame, int totalFrames,
                    const std::string &frameFilename);
void checkBenchmark(OmpRenderer *ref_renderer, RefRenderer *cuda_renderer,
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

    TreeRenderer *renderer;
    int numberOfTrees = 5000;
    int threads = 8;
    int depth = 5;
    float length = .01;
    
    // Set args
    int opt;
    while((opt = getopt(argc, argv,"t:d:l:n:")) != EOF) {
        switch(opt) {
            case 't':
                numberOfTrees = atoi(optarg);
                break;
            case 'd':
                depth = atoi(optarg);
                break;
            case 'l':
                length = atoi(optarg);
                break;
            case 'n':
                threads = atoi(optarg);
        } 
    }

    LSystem trees[numberOfTrees];

    double preGenerate = CycleTimer::currentSeconds();
    for (int i = 0; i < numberOfTrees ; i++) {

        float x = randomFloat(), y = randomFloat(); // initial position
        float angle = 3.14/2; // initial angle
        float colors[3] = {randomFloat()*0.f + 1.0f, randomFloat()*1.0f + 0.0f, randomFloat()*0.5f + 0.0f};
        Fern L(x, y, angle, length/(1<<depth), colors);
        L.generate(depth);
        trees[i] = L;
    }
    double endGenerate = CycleTimer::currentSeconds();

    printf("Create %d trees with depth %d, length %.3f in %.3f ms\n", numberOfTrees, depth, length, 1000.f * (endGenerate - preGenerate));

    renderer = new OmpRenderer();

    renderer->allocOutputImage(imageSize, imageSize);
    renderer->loadTrees(trees, numberOfTrees);
    renderer->setup(threads);

    glutInit(&argc, argv);
    startRendererWithDisplay(renderer);

    return 0;
}
