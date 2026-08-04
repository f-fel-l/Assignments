#include "Scene3D.h"

#ifdef PLATFORM_ANDROID
Scene3D::Scene3D(AAssetManager* assetMgr) {
    fan = new Fan(assetMgr);
    ground = new Ground(assetMgr);
}
#else
Scene3D::Scene3D() {
    fan = new Fan();
    ground = new Ground();
}
#endif

Scene3D::~Scene3D() {
    delete fan;
    delete ground;
}

void Scene3D::InitModel() {
    if (fan) fan->InitModel();
    if (ground) ground->InitModel();
}

void Scene3D::Render() {
    // To be implemented fully in Part 1 & Part 2
    if (fan) fan->Render();
    if (ground) ground->Render();
}

void Scene3D::Resize(int w, int h) {
    screenWidth = w;
    screenHeight = h;
    if (fan) fan->Resize(w, h);
    if (ground) ground->Resize(w, h);
}

void Scene3D::SetCameraDistance(float d) {
    cameraDistance = d;
    if (fan) fan->SetCameraDistance(d);
    if (ground) ground->SetCameraDistance(d);
}

bool Scene3D::PickAt(float screenX, float screenY) {
    if (fan) {
        return fan->PickAt(screenX, screenY, screenWidth, screenHeight);
    }
    return false;
}

void Scene3D::TouchEventDown(float x, float y) {
    if (PickAt(x, y)) {
        LOGI("Scene3D: Fan part successfully picked!");
    }
}

void Scene3D::TouchEventMove(float x, float y) {
    if (fan) fan->TouchEventMove(x, y);
}

void Scene3D::TouchEventRelease(float x, float y) {
    if (fan) fan->TouchEventRelease(x, y);
}

void Scene3D::SetFanSpeed(float s) {
    if (fan) fan->SetSpeed(s);
}