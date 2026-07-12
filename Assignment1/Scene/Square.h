#pragma once

/**
 * Square.h
 *
 * Self-contained square renderer (shader, VAO, VBOs), rendered alongside
 * Triangle to prove the Model + Renderer architecture scales to more than
 * one shape. Owns its own GPU resources — it does not share Triangle's
 * program/VAO/VBOs, even though it reuses the same .glsl shader files.
 * Used by Desktop (Windows/GLFW) and Emscripten builds.
 * Android uses NativeTemplate.cpp (monolithic JNI) instead.
 */

#include "Platform.h"
#include "Model.h"

// Bring in the AAssetManager definition for Android
#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Square : public Model {
public:
#ifdef PLATFORM_ANDROID
    explicit Square(AAssetManager* assetMgr);
#else
    Square();
#endif
    ~Square() override;

    void InitModel() override;
    void Render() override;
    void Resize(int w, int h) override;

private:
#ifdef PLATFORM_ANDROID
    AAssetManager* mgr = nullptr;
#endif
    GLuint programID    = 0;
    GLuint vao          = 0;
    GLuint vboPos       = 0;
    GLuint vboColor     = 0;
    GLint  uRadianAngle = -1;
};
