/**
 * Fan.cpp
 *
 * Implements the part hierarchy and gesture behaviour described in Fan.h:
 * one shared cube VAO drawn seven times (base, pole, hub, four blades) with
 * a per-part model matrix composed on the Transform push/pop stack, each
 * tinted through the PARTCOLOR uniform. TouchEventDown/Move/Release
 * distinguish a tap (toggles fanOn) from a swipe (drives dragBoost from the
 * measured drag velocity).
 */
#define LOG_TAG "Fan"
#include "Fan.h"
#include "ShaderHelper.h"
#include <glm/glm.hpp>
#include <cmath>
#include <algorithm>

// 8-vertex cube
static const GLfloat kCubePositions[] = {
        // Front face (z = 0.5)
        -0.5f, -0.5f,  0.5f,   0.5f, -0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,  -0.5f,  0.5f,  0.5f,
        // Back face (z = -0.5)
        -0.5f, -0.5f, -0.5f,   0.5f, -0.5f, -0.5f,
        0.5f,  0.5f, -0.5f,  -0.5f,  0.5f, -0.5f
};

// Shade factor: front corners 1.0, back corners 0.6
static const GLfloat kCubeShades[] = {
        1.0f, 1.0f, 1.0f, 1.0f,
        0.6f, 0.6f, 0.6f, 0.6f
};

// 36 indices
static const GLuint kCubeIndices[] = {
        0,1,2, 2,3,0, // Front
        1,5,6, 6,2,1, // Right
        7,6,5, 5,4,7, // Back
        4,0,3, 3,7,4, // Left
        3,2,6, 6,7,3, // Top
        4,5,1, 1,0,4  // Bottom
};

#ifdef PLATFORM_ANDROID
Fan::Fan(AAssetManager* assetMgr) : mgr(assetMgr) { modelType = CubeType; }
#else
Fan::Fan() { modelType = CubeType; }
#endif

Fan::~Fan() {
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (ibo) glDeleteBuffers(1, &ibo);
    if (program) glDeleteProgram(program);
}

void Fan::InitModel() {
    transform.TransformInit();

#ifdef PLATFORM_ANDROID
    program = ShaderHelper::buildProgramFromAssets(mgr, "shader/FanVertex.glsl", "shader/FanFragment.glsl");
#else
    program = ShaderHelper::buildProgramFromFile("FanVertex.glsl", "FanFragment.glsl");
#endif

    if (!program) { LOGE("Fan: failed to build shader program"); return; }

    uMVP = glGetUniformLocation(program, "uMVP");
    uPartColor = glGetUniformLocation(program, "PARTCOLOR");

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kCubePositions) + sizeof(kCubeShades), nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(kCubePositions), kCubePositions);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(kCubePositions), sizeof(kCubeShades), kCubeShades);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(kCubePositions)));

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kCubeIndices), kCubeIndices, GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void Fan::Resize(int w, int h) {
    float aspect = (float)w / (float)h;
    transform.TransformSetMatrixMode(PROJECTION_MATRIX);
    transform.TransformLoadIdentity();
    transform.TransformSetPerspective(glm::radians(60.0f), aspect, 0.01f, 1000.0f, 0);
}

void Fan::Render() {
    if (!program || !vao) return;

    if (fanOn) {
        spinAngle += (kBaseSpeed + dragBoost);
        if (spinAngle >= 360.0f) spinAngle -= 360.0f;
    }

    glUseProgram(program);
    glBindVertexArray(vao);
    glEnable(GL_DEPTH_TEST);

    transform.TransformSetMatrixMode(MODEL_MATRIX);
    transform.TransformLoadIdentity();

    // World pull-back and tilt
    transform.TransformTranslate(0.0f, 0.8f, -8.0f);
    transform.TransformRotate(glm::radians(20.0f), 0.0f, 1.0f, 0.0f);

    auto drawPart = [&](float r, float g, float b) {
        glUniform3f(uPartColor, r, g, b);

        // Use a direct float pointer cast instead of value_ptr
        glUniformMatrix4fv(uMVP, 1, GL_FALSE, (const GLfloat*)transform.TransformGetModelViewProjectionMatrix());

        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    };

    // Base
    transform.TransformPushMatrix();
    transform.TransformTranslate(0.0f, -2.6f, 0.0f);
    transform.TransformScale(1.6f, 0.25f, 0.8f);
    drawPart(0.45f, 0.28f, 0.12f); // Brown
    transform.TransformPopMatrix();

    // Pole
    transform.TransformPushMatrix();
    transform.TransformTranslate(0.0f, -1.21f, 0.0f);
    transform.TransformScale(0.15f, 2.53f, 0.15f);
    drawPart(0.55f, 0.55f, 0.58f); // Light gray
    transform.TransformPopMatrix();

    // Hub
    transform.TransformPushMatrix();
    transform.TransformTranslate(0.0f, 0.2f, 0.0f);
    transform.TransformScale(0.3f, 0.3f, 0.3f);
    drawPart(0.20f, 0.20f, 0.22f); // Dark gray
    transform.TransformPopMatrix();

    // Blades: each pivots off the hub centre (0, 0.2, 0.15), rotates by
    // spinAngle + i*90 around Z, then offsets outward before drawing.
    glm::vec3 bladeColors[4] = {
            {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f},
            {1.0f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}
    };
    for (int i = 0; i < 4; i++) {
        transform.TransformPushMatrix();
        transform.TransformTranslate(0.0f, 0.2f, 0.15f);
        transform.TransformRotate(glm::radians(spinAngle + (i * 90.0f)), 0.0f, 0.0f, 1.0f);
        transform.TransformTranslate(0.0f, 0.55f, 0.0f);
        transform.TransformScale(0.22f, 0.8f, 0.05f);
        drawPart(bladeColors[i].r, bladeColors[i].g, bladeColors[i].b);
        transform.TransformPopMatrix();
    }
}

void Fan::TouchEventDown(float x, float y) {
    lastX = x; lastY = y;
    movedDistance = 0.0f;
    lastMoveTime = std::chrono::steady_clock::now();
}

void Fan::TouchEventMove(float x, float y) {
    float dx = x - lastX;
    float dy = y - lastY;
    movedDistance += std::hypot(dx, dy);

    auto now = std::chrono::steady_clock::now();
    float dtMs = std::chrono::duration<float, std::milli>(now - lastMoveTime).count();

    if (dtMs > 0.0f) {
        float velocity = std::hypot(dx, dy) / dtMs;
        dragBoost = std::min(velocity * kBoostScale, kMaxBoost);
    }

    lastX = x; lastY = y;
    lastMoveTime = now;
}

void Fan::TouchEventRelease(float x, float y) {
    if (movedDistance < kTapThreshold) {
        fanOn = !fanOn;
        LOGI("Fan %s", fanOn ? "ON" : "OFF");
    }
    dragBoost = 0.0f;
}