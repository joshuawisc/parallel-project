#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "platformgl.h"
#include "refRenderer.h"

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

    int numberOfTrees = 1330000;

    float* position = new float[3*numberOfTrees];
    float* color = new float[3*numberOfTrees];

    for (int i = 0; i < numberOfTrees ; i++) {
        int index3 = 3 * i;

        position[index3] = randomFloat();
        position[index3 + 1] = randomFloat();
        position[index3 + 2] = randomFloat();

        color[index3] = randomFloat();
        color[index3 + 1] = randomFloat();
        color[index3 + 2] = randomFloat();
    }


    renderer = new RefRenderer();

    renderer->allocOutputImage(imageSize, imageSize);
    renderer->loadTrees(position, color, numberOfTrees);
    renderer->setup();

    glutInit(&argc, argv);
    startRendererWithDisplay(renderer);

    return 0;
}
