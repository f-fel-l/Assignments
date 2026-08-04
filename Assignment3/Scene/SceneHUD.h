#pragma once
#include "Model.h"
#include "Transform.h"

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

// simple rectangle struct for pixel coordinates
struct HUDElement {
    float x, y, w, h;
};

class SceneHUD : public Model {
public:
#ifdef PLATFORM_ANDROID
    explicit SceneHUD(AAssetManager* assetMgr);
#else
    SceneHUD();
#endif
    ~SceneHUD() override;

    void InitModel() override;
    void Render() override;
    void Resize(int w, int h) override;
    bool TouchDown(float x, float y);
    void TouchRelease();

    float CurrentSpeed() const { return speed; }

private:
#ifdef PLATFORM_ANDROID
    AAssetManager* mgr = nullptr;
#endif

    GLuint program = 0;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ibo = 0;

    GLint uMVP = -1;
    GLint uColor = -1;

    Transform transform;

    // HUD element bounds
    HUDElement btnPlusSpd;
    HUDElement btnMinusSpd;
    HUDElement btnPlusZoom;
    HUDElement btnMinusZoom;
    HUDElement speedBar[20];

    float speed = 8.0f;
    float zoom = 8.0f; // default camera distance
    int activeButton = -1;

    // smooth easing
    float targetSpeed = 8.0f;
    float targetZoom = 8.0f;

public:
    float CurrentZoom() const { return zoom; } // getter for Renderer
};