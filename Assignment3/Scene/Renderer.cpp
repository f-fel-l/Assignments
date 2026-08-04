#define LOG_TAG "Renderer"
#include "Renderer.h"
#include "Scene3D.h"
#include "SceneHUD.h"

Renderer::~Renderer() { clearModels(); }

#ifdef PLATFORM_ANDROID
void Renderer::setAssetManager(AAssetManager* mgr) { assetMgr = mgr; }
#endif

bool Renderer::initializeRenderer()
{
    LOGI("Renderer::initializeRenderer");
    createModels();
    initializeModels();
    return true;
}

void Renderer::createModels()
{
    clearModels();
#ifdef PLATFORM_ANDROID
    scene3D = new Scene3D(assetMgr);     // <--- Make sure this line is still here!
    sceneHUD = new SceneHUD(assetMgr);
#else
    scene3D = new Scene3D();             // <--- And this one!
    sceneHUD = new SceneHUD();
#endif
    LOGI("Renderer: Scene3D and SceneHUD created");
}

void Renderer::initializeModels()
{
    if (scene3D) scene3D->InitModel();
    if (sceneHUD) sceneHUD->InitModel();
}

void Renderer::clearModels()
{
    delete scene3D;
    scene3D = nullptr;
    delete sceneHUD;
    sceneHUD = nullptr;
}

void Renderer::resize(int w, int h)
{
    screenWidth  = w;
    screenHeight = h;
    glViewport(0, 0, w, h);

    if (scene3D) scene3D->Resize(w, h);
    if (sceneHUD) sceneHUD->Resize(w, h);

    LOGI("Renderer::resize %d x %d", w, h);
}

void Renderer::render()
{
    // sync shared state from HUD to Scene3D before drawing
    if (sceneHUD && scene3D) {
        scene3D->SetFanSpeed(sceneHUD->CurrentSpeed());
        scene3D->SetCameraDistance(sceneHUD->CurrentZoom());
    }

    // clear Framebuffer
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // render Scene3D
    glEnable(GL_DEPTH_TEST);
    if (scene3D) scene3D->Render();

    // disable Depth Test
    glDisable(GL_DEPTH_TEST);

    // Render SceneHUD
    if (sceneHUD) sceneHUD->Render();

    // Enable Depth Test
    glEnable(GL_DEPTH_TEST);
}

void Renderer::TouchEventDown(float x, float y)
{
    // Route to HUD first, if HUD doesn't consume, pass to Scene3D
    if (sceneHUD && sceneHUD->TouchDown(x, y)) {
        return;
    }
    if (scene3D) scene3D->TouchEventDown(x, y);
}

void Renderer::TouchEventMove(float x, float y)
{
    if (scene3D) scene3D->TouchEventMove(x, y);
}

void Renderer::TouchEventRelease(float x, float y)
{
    if (sceneHUD) sceneHUD->TouchRelease();
    if (scene3D) scene3D->TouchEventRelease(x, y);
}
