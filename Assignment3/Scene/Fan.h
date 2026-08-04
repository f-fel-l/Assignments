#pragma once

/**
 * Fan.h
 *
 * A 3D table fan built from a single 8-vertex cube, drawn seven times with a
 * different model matrix per part on the Transform push/pop stack
 * Each part is tinted with its own colour - the cube geometry/shade is shared by all seven.
 *
 */

#include "Model.h"
#include "Platform.h"
#include "Transform.h"
#include <chrono>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Fan : public Model {
public:
#ifdef PLATFORM_ANDROID
    explicit Fan(AAssetManager* assetMgr);
#else
    Fan();
#endif
    ~Fan() override;

    void InitModel() override;
    void Render() override;
    void Resize(int w, int h) override;

    void TouchEventDown(float x, float y) override;
    void TouchEventMove(float x, float y) override;
    void TouchEventRelease(float x, float y) override;

    // Quick public setter for Scene3D to use
    void SetSpeed(float s) { currentSpeed = s; }
    void SetCameraDistance(float d) { cameraDistance = d; }
    bool PickAt(float x, float y, int screenW, int screenH);

private:
#ifdef PLATFORM_ANDROID
    AAssetManager* mgr = nullptr;
#endif

    GLuint program = 0;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ibo = 0;

    GLint uMVP = -1;
    GLint uPartColor = -1;

    GLint uModel = -1;
    GLint uNormalMatrix = -1;
    GLint uLightPos = -1;
    GLint uViewPos = -1;

    Transform transform;

    float spinAngle = 0.0f;
    float currentSpeed = 8.0f; // Default starting speed

    float cameraDistance = 8.0f;

    // structures to cache world positions for picking
    struct PartData {
        glm::vec3 center;
        glm::vec3 tip;
        bool isSegment;
        float radius;
    };
    PartData parts[7];     // Base, Pole, Hub, 4x Blades
    int selectedPart = -1; // -1 = nothing selected

    glm::mat4 cachedView;
    glm::mat4 cachedProj;
};