#pragma once

/**
 * Triangle.h
 *
 * Renders a continuously rotating 2D triangle using OpenGL ES 3.0 VAO/VBO.
 * Each vertex has a distinct color (red, green, blue).
 * A RadianAngle uniform drives a 2D rotation matrix in the vertex shader.
 *
 * This is the introductory GLPI Framework demo – no touch interaction.
 * Touch events are introduced in 4-TouchEvents-Modern.
 *
 * Platform-agnostic – shared by Android, Desktop, and WebGL builds.
 */

#include "Model.h"
#include "Platform.h"

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Triangle : public Model {
public:
#ifdef PLATFORM_ANDROID
    explicit Triangle(AAssetManager* assetMgr);
#else
    Triangle();
#endif
    ~Triangle() override;

    void InitModel() override;
    void Render()    override;

private:
#ifdef PLATFORM_ANDROID
    AAssetManager* mgr = nullptr;
#endif

    GLuint program      = 0;
    GLuint vao          = 0;
    GLuint vboPos       = 0;
    GLuint vboColor     = 0;

    // Rotation state
    float  degree       = 0.0f;
    GLint  uRadianAngle = -1;
};
