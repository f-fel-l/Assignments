#pragma once

/**
 * Renderer.h
 *
 * Singleton renderer that owns all Model instances and drives the render loop.
 * Platform-agnostic – shared by Android, Desktop, and WebGL builds.
 */

#include "Model.h"
#include "Platform.h"

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

// new scenes
class Scene3D;
class SceneHUD;

class Renderer {
public:
    static Renderer& Instance() {
        static Renderer instance;
        return instance;
    }

    Renderer(const Renderer&)            = delete;
    Renderer& operator=(const Renderer&) = delete;

#ifdef PLATFORM_ANDROID
    void setAssetManager(AAssetManager* mgr);
#endif

    bool initializeRenderer();
    void resize(int w, int h);
    void render();

    void TouchEventDown(float x, float y);
    void TouchEventMove(float x, float y);
    void TouchEventRelease(float x, float y);

private:
    Renderer()  = default;
    ~Renderer();

    void createModels();
    void initializeModels();
    void clearModels();

#ifdef PLATFORM_ANDROID
    AAssetManager* assetMgr = nullptr;
#endif

    Scene3D* scene3D = nullptr;
    SceneHUD* sceneHUD = nullptr;

    int screenWidth  = 0;
    int screenHeight = 0;
};
