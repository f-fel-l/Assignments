#pragma once
#include "Model.h"
#include "Fan.h"
#include "Ground.h"

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Scene3D : public Model {
public:
#ifdef PLATFORM_ANDROID
    explicit Scene3D(AAssetManager* assetMgr);
#else
    Scene3D();
#endif
    ~Scene3D() override;

    void InitModel() override;
    void Render() override;
    void Resize(int w, int h) override;
    void SetCameraDistance(float d);
    bool PickAt(float screenX, float screenY);

    void TouchEventDown(float x, float y) override;
    void TouchEventMove(float x, float y) override;
    void TouchEventRelease(float x, float y) override;

    void SetFanSpeed(float s);

private:
    Fan* fan = nullptr;
    Ground* ground = nullptr;
    float cameraDistance = 8.0f;
    int screenWidth = 0;
    int screenHeight = 0;
};