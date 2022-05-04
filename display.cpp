#include <algorithm>

#include "cycleTimer.h"
#include "image.h"
#include "platformgl.h"
#include "treeRenderer.h"

void renderPicture();

static struct {
    int width;
    int height;
    bool updateSim;
    bool printStats;
    bool pauseSim;
    double lastFrameTime;

    TreeRenderer *renderer;

} gDisplay;

// handleReshape --
//
// Event handler, fired when the window is resized
void handleReshape(int w, int h) {
    gDisplay.width = w;
    gDisplay.height = h;
    glViewport(0, 0, gDisplay.width, gDisplay.height);
    glutPostRedisplay();
}

void handleDisplay() {
    // Simulation and rendering work is done in the renderPicture
    // function below
    renderPicture();

    // The subsequent code uses OpenGL to present the state of the
    // rendered image on the screen.
    const Image *img = gDisplay.renderer->getImage();

    int width = std::min(img->width, gDisplay.width);
    int height = std::min(img->height, gDisplay.height);

    glDisable(GL_DEPTH_TEST);
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.f, gDisplay.width, 0.f, gDisplay.height, -1.f, 1.f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Copy image data from the renderer to the OpenGL
    // frame-buffer.  This is inefficient solution as the processing
    // to generate the image is done in CUDA.  An improved solution
    // would render to a CUDA surface object (stored in GPU memory),
    // and then bind this surface as a texture enabling it's use in
    // normal OpenGL rendering
    glRasterPos2i(0, 0);
    glDrawPixels(width, height, GL_RGBA, GL_FLOAT, img->data);

    double currentTime = CycleTimer::currentSeconds();

    if (gDisplay.printStats)
        printf("%.2f ms\n", 1000.f * (currentTime - gDisplay.lastFrameTime));

    gDisplay.lastFrameTime = currentTime;

    glutSwapBuffers();
    glutPostRedisplay();
}

// handleKeyPress --
//
// Keyboard event handler
void handleKeyPress(unsigned char key, int x, int y) {

    switch (key) {
    case 'q':
    case 'Q':
        exit(1);
        break;
    case '=':
    case '+':
        gDisplay.updateSim = true;
        break;
    case 'p':
    case 'P':
        printf("Pause\n");
        gDisplay.pauseSim = !gDisplay.pauseSim;
        if (!gDisplay.pauseSim)
            gDisplay.updateSim = true;
        break;
    }
}

// renderPicture --
//
// At the reall work is done here, not in the display handler
void renderPicture() {

    double startTime = CycleTimer::currentSeconds();


    double endClearTime = CycleTimer::currentSeconds();

    // Update particle positions and state
    if (gDisplay.updateSim) {
        gDisplay.renderer->advanceAnimation();
    }
    if (gDisplay.pauseSim)
        gDisplay.updateSim = false;

    double endSimTime = CycleTimer::currentSeconds();

    if (!gDisplay.pauseSim) {
        // Render the particles into the image
        // Clear screen
        gDisplay.renderer->clearImage();
        gDisplay.renderer->render();
        if (gDisplay.printStats) {
            printf("Clear:    %.3f ms\n", 1000.f * (endClearTime - startTime));
            printf("Advance:  %.3f ms\n", 1000.f * (endSimTime - endClearTime));
            printf("Render:   %.3f ms\n", 1000.f * (CycleTimer::currentSeconds() - endSimTime));
        }
    }


}

void startRendererWithDisplay(TreeRenderer *renderer, int threads) {

    // Setup the display
    double preStart = CycleTimer::currentSeconds();
    renderer->setup(threads);
    double postSetup = CycleTimer::currentSeconds();
    const Image *img = renderer->getImage();
    double postGet = CycleTimer::currentSeconds();

    double setupTime = 1000.f*(postSetup - preStart);
    double getTime = 1000.f * (postGet - postSetup);

    gDisplay.renderer = renderer;
    gDisplay.updateSim = true;
    gDisplay.pauseSim = true;
    gDisplay.printStats = false;
    gDisplay.lastFrameTime = CycleTimer::currentSeconds();
    gDisplay.width = img->width;
    gDisplay.height = img->height;

    // Configure GLUT

    glutInitWindowSize(gDisplay.width, gDisplay.height);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
    glutCreateWindow("CMU 15-618 Parallel Project Trees");
    glutDisplayFunc(handleDisplay);
    glutKeyboardFunc(handleKeyPress);
    double preTime = CycleTimer::currentSeconds();
    gDisplay.renderer->clearImage();
    double postClear = CycleTimer::currentSeconds();
    gDisplay.renderer->render();
    double postRender = CycleTimer::currentSeconds();
    double clearTime = 1000.f*(postClear - preTime);
    double renderTime = 1000.f*(postRender - postClear);
    double totalTime = 1000.f * (postGet - preStart + postRender - preTime);
    printf("---------------------DISPLAY TIMES--------------------\n");
    printf("First setup:\t%.3fms\nFirst get:\t%.3f ms\nFirst clear:\t%.3f ms\nFirst render:\t%.3f ms\nTotal:\t\t%.3f ms\n", setupTime, getTime, clearTime, renderTime, totalTime);
    printf("-----------------------------------------------------\n");
    glutMainLoop();
}
