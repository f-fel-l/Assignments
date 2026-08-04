#include "SceneHUD.h"
#include "ShaderHelper.h"
#include <cmath>

// simple 1x1 Unit Quad (Top-Left origin)
static const GLfloat kQuadVertices[] = {
        0.0f, 0.0f,  // Top-Left
        1.0f, 0.0f,  // Top-Right
        1.0f, 1.0f,  // Bottom-Right
        0.0f, 1.0f   // Bottom-Left
};

static const GLuint kQuadIndices[] = { 0, 1, 2, 2, 3, 0 };

// function to check if a point is inside AABB
static bool PointInRect(float px, float py, const HUDElement& rect) {
    return (px >= rect.x && px <= rect.x + rect.w &&
            py >= rect.y && py <= rect.y + rect.h);
}

#ifdef PLATFORM_ANDROID
SceneHUD::SceneHUD(AAssetManager* assetMgr) : mgr(assetMgr) { modelType = TriangleType; }
#else
SceneHUD::SceneHUD() { modelType = TriangleType; }
#endif

SceneHUD::~SceneHUD() {
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (ibo) glDeleteBuffers(1, &ibo);
    if (program) glDeleteProgram(program);
}

void SceneHUD::InitModel() {
    transform.TransformInit();

#ifdef PLATFORM_ANDROID
    program = ShaderHelper::buildProgramFromAssets(mgr, "shader/HUDVertex.glsl", "shader/HUDFragment.glsl");
#else
    program = ShaderHelper::buildProgramFromFile("HUDVertex.glsl", "HUDFragment.glsl");
#endif

    uMVP = glGetUniformLocation(program, "uMVP");
    uColor = glGetUniformLocation(program, "uColor");

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kQuadVertices), kQuadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kQuadIndices), kQuadIndices, GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void SceneHUD::Resize(int w, int h) {
    transform.TransformSetMatrixMode(PROJECTION_MATRIX);
    transform.TransformLoadIdentity();
    // top-Left origin
    transform.TransformOrtho(0.0f, (float)w, (float)h, 0.0f, -1.0f, 1.0f);

    // calculate layout in pixels
    float btnW = 140.0f;
    float btnH = 70.0f;
    float margin = 40.0f;

    // vehicle Controls (Left)
    btnPlusSpd  = { margin, margin, btnW, btnH };
    btnMinusSpd = { margin, margin + btnH + 20.0f, btnW, btnH };

    // camera Controls (Right)
    btnPlusZoom  = { (float)w - margin - btnW, margin, btnW, btnH };
    btnMinusZoom = { (float)w - margin - btnW, margin + btnH + 20.0f, btnW, btnH };

    // speed Indicator (Bottom Center)
    float barW = 15.0f;
    float barH = 30.0f;
    float totalBarW = 20.0f * barW;
    float startX = ((float)w - totalBarW) / 2.0f;
    float startY = (float)h - margin - barH;

    for (int i = 0; i < 20; i++) {
        speedBar[i] = { startX + (i * barW), startY, barW - 2.0f, barH };
    }
}

void SceneHUD::Render() {
    if (!program || !vao) return;

    // BONUS 1 & 2: Press-and-Hold + Smooth Easing

    // continuously adjust targets while button is held down
    float holdRate = 0.1f; // Speed of continuous adjustment per frame
    if (activeButton == 1) targetSpeed = std::min(targetSpeed + holdRate, 20.0f);
    if (activeButton == 2) targetSpeed = std::max(targetSpeed - holdRate, 0.0f);
    if (activeButton == 3) targetZoom  = std::max(targetZoom - holdRate, 3.0f);
    if (activeButton == 4) targetZoom  = std::min(targetZoom + holdRate, 20.0f);

    // move actual values 15% towards the target every frame
    speed += (targetSpeed - speed) * 0.15f;
    zoom += (targetZoom - zoom) * 0.15f;

    // snap to target if very close
    if (std::abs(targetSpeed - speed) < 0.01f) speed = targetSpeed;
    if (std::abs(targetZoom - zoom) < 0.01f) zoom = targetZoom;

    glUseProgram(program);
    glBindVertexArray(vao);
    glDisable(GL_DEPTH_TEST); // ensure HUD draws over everything
    glDisable(GL_CULL_FACE);

    transform.TransformSetMatrixMode(MODEL_MATRIX);

    // draw colored rectangle using Unit Quad
    auto drawRect = [&](const HUDElement& rect, float r, float g, float b) {
        transform.TransformLoadIdentity();
        transform.TransformTranslate(rect.x, rect.y, 0.0f);
        transform.TransformScale(rect.w, rect.h, 1.0f);

        glUniformMatrix4fv(uMVP, 1, GL_FALSE, (const GLfloat*)transform.TransformGetModelViewProjectionMatrix());
        glUniform4f(uColor, r, g, b, 1.0f);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    };

    // draw buttons
    drawRect(btnPlusSpd,  0.2f, 0.6f, 1.0f); // Blue
    drawRect(btnMinusSpd, 0.2f, 0.6f, 1.0f); // Blue
    drawRect(btnPlusZoom, 1.0f, 0.5f, 0.0f); // Orange
    drawRect(btnMinusZoom,1.0f, 0.5f, 0.0f); // Orange

    // Draw the speed bar (Dynamic: Green for active, Gray for inactive)
    for (int i = 0; i < 20; i++) {
        if (i < (int)speed) {
            drawRect(speedBar[i], 0.0f, 0.8f, 0.0f); // Green
        } else {
            drawRect(speedBar[i], 0.3f, 0.3f, 0.3f); // Gray
        }
    }
}

bool SceneHUD::TouchDown(float x, float y) {
    if (PointInRect(x, y, btnPlusSpd)) {
        activeButton = 1;
        targetSpeed = std::min(targetSpeed + 1.0f, 20.0f);
        LOGI("+SPD pressed");
        return true;
    }
    if (PointInRect(x, y, btnMinusSpd)) {
        activeButton = 2;
        targetSpeed = std::max(targetSpeed - 1.0f, 0.0f);
        LOGI("-SPD pressed");
        return true;
    }
    if (PointInRect(x, y, btnPlusZoom)) {
        activeButton = 3;
        targetZoom = std::max(targetZoom - 1.0f, 3.0f); // moves closer
        LOGI("+ZOOM pressed");
        return true;
    }
    if (PointInRect(x, y, btnMinusZoom)) {
        activeButton = 4;
        targetZoom = std::min(targetZoom + 1.0f, 20.0f); // moves further
        LOGI("-ZOOM pressed");
        return true;
    }
    return false;
}

void SceneHUD::TouchRelease() { activeButton = -1; }