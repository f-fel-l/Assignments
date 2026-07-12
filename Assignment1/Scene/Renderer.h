#pragma once

/**
 * Renderer.h
 *
 * Singleton controller that owns every Model in the scene through the Model
 * interface only. main.cpp / NativeTemplate.cpp talk to Renderer exclusively —
 * neither of them (nor this header) ever needs to know which concrete shapes
 * (Triangle, Square, ...) actually exist; that knowledge is confined to
 * Renderer.cpp's createModels().
 */

#include "Model.h"
#include <vector>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Renderer {
public:
    static Renderer& Instance();
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

#ifdef PLATFORM_ANDROID
    void setAssetManager(AAssetManager* mgr);
#endif

    bool initializeRenderer();
    void resize(int w, int h);
    void render();

private:
    Renderer() = default;
    ~Renderer();

    void createModels();
    void initializeModels();
    void clearModels();

#ifdef PLATFORM_ANDROID
    AAssetManager* assetMgr = nullptr;
#endif
    std::vector<Model*> models; // replaces the single Model* model
};