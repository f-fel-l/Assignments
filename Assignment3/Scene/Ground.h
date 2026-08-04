#pragma once
#include "Model.h"
#include "Transform.h"

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Ground : public Model {
public:
#ifdef PLATFORM_ANDROID
    explicit Ground(AAssetManager* assetMgr);
#else
    Ground();
#endif
    ~Ground() override;

    void InitModel() override;
    void Render() override;
    void Resize(int w, int h) override;

    void SetCameraDistance(float d) { cameraDistance = d; }

private:
#ifdef PLATFORM_ANDROID
    AAssetManager* mgr = nullptr;
#endif

    GLuint program = 0;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ibo = 0;

    GLint uMVP = -1;
    GLint uModel = -1;
    GLint uNormalMatrix = -1;
    GLint uLightPos = -1;
    GLint uViewPos = -1;

    Transform transform;

    float cameraDistance = 8.0f;
};