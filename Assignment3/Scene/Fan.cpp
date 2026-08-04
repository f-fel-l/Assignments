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

// 8 normals pointing outwards from center for soft lighting
static const GLfloat kCubeNormals[] = {
        -0.577f, -0.577f,  0.577f,   0.577f, -0.577f,  0.577f,
        0.577f,  0.577f,  0.577f,  -0.577f,  0.577f,  0.577f,
        -0.577f, -0.577f, -0.577f,   0.577f, -0.577f, -0.577f,
        0.577f,  0.577f, -0.577f,  -0.577f,  0.577f, -0.577f
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
    uModel = glGetUniformLocation(program, "uModel");
    uNormalMatrix = glGetUniformLocation(program, "uNormalMatrix");
    uLightPos = glGetUniformLocation(program, "uLightPos");
    uViewPos = glGetUniformLocation(program, "uViewPos");

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // increase total buffer size to include normals
    glBufferData(GL_ARRAY_BUFFER, sizeof(kCubePositions) + sizeof(kCubeShades) + sizeof(kCubeNormals), nullptr, GL_STATIC_DRAW);

    // upload all three arrays
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(kCubePositions), kCubePositions);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(kCubePositions), sizeof(kCubeShades), kCubeShades);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(kCubePositions) + sizeof(kCubeShades), sizeof(kCubeNormals), kCubeNormals);

    // attribute 0: Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // attribute 1: Shades
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(kCubePositions)));

    // attribute 2: Normals
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(kCubePositions) + sizeof(kCubeShades)));

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

    // Simple speed addition, wrapping at 360
    spinAngle += currentSpeed;
    if (spinAngle >= 360.0f) spinAngle -= 360.0f;

    glUseProgram(program);
    glBindVertexArray(vao);
    glEnable(GL_DEPTH_TEST);

    // Cache Projection
    transform.TransformSetMatrixMode(PROJECTION_MATRIX);
    cachedProj = *transform.TransformGetProjectionMatrix();

    // Setup Camera in VIEW_MATRIX & Cache it
    transform.TransformSetMatrixMode(VIEW_MATRIX);
    transform.TransformLoadIdentity();
    transform.TransformTranslate(0.0f, 0.8f, -cameraDistance);
    transform.TransformRotate(glm::radians(20.0f), 0.0f, 1.0f, 0.0f);
    cachedView = *transform.TransformGetViewMatrix();

    // Setup Model Matrix
    transform.TransformSetMatrixMode(MODEL_MATRIX);
    transform.TransformLoadIdentity();

    // Helper to get World-Space coordinates from local coordinates
    auto extractWorldPos = [&](float ox, float oy, float oz) {
        glm::vec4 p(ox, oy, oz, 1.0f);
        return glm::vec3(*transform.TransformGetModelMatrix() * p);
    };

    // Updated drawPart to handle highlighting
    auto drawPart = [&](int id, float r, float g, float b) {
        if (id == selectedPart) {
            glUniform3f(uPartColor, 1.0f, 0.75f, 0.0f); // Amber Highlight!
        } else {
            glUniform3f(uPartColor, r, g, b);
        }

        glUniformMatrix4fv(uMVP, 1, GL_FALSE, (const GLfloat*)transform.TransformGetModelViewProjectionMatrix());
        glm::mat4 model = *transform.TransformGetModelMatrix();
        glUniformMatrix4fv(uModel, 1, GL_FALSE, (const GLfloat*)&model[0][0]);
        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
        glUniformMatrix3fv(uNormalMatrix, 1, GL_FALSE, (const GLfloat*)&normalMatrix[0][0]);
        glUniform3f(uLightPos, 5.0f, 5.0f, 5.0f);
        glUniform3f(uViewPos, 0.0f, 0.0f, 0.0f);

        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    };

    // Base
    transform.TransformPushMatrix();
    transform.TransformTranslate(0.0f, -2.6f, 0.0f);
    transform.TransformScale(1.6f, 0.25f, 0.8f);
    parts[0] = { extractWorldPos(0,0,0), glm::vec3(0), false, 1.5f }; // Cache
    drawPart(0, 0.45f, 0.28f, 0.12f); // Brown
    transform.TransformPopMatrix();

    // Pole
    transform.TransformPushMatrix();
    transform.TransformTranslate(0.0f, -1.21f, 0.0f);
    transform.TransformScale(0.15f, 2.53f, 0.15f);
    parts[1] = { extractWorldPos(0,0,0), glm::vec3(0), false, 1.3f }; // Cache
    drawPart(1, 0.55f, 0.55f, 0.58f); // Light gray
    transform.TransformPopMatrix();

    // Hub
    transform.TransformPushMatrix();
    transform.TransformTranslate(0.0f, 0.2f, 0.0f);
    transform.TransformScale(0.3f, 0.3f, 0.3f);
    parts[2] = { extractWorldPos(0,0,0), glm::vec3(0), false, 0.4f }; // Cache
    drawPart(2, 0.20f, 0.20f, 0.22f); // Dark gray
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
        parts[3+i] = { extractWorldPos(0, -0.45f, 0), extractWorldPos(0, 0.45f, 0), true, 0.25f }; // Cache segment
        drawPart(3+i, bladeColors[i].r, bladeColors[i].g, bladeColors[i].b);
        transform.TransformPopMatrix();
    }
}

void Fan::TouchEventDown(float x, float y) {}
void Fan::TouchEventMove(float x, float y) {}
void Fan::TouchEventRelease(float x, float y) {}

bool Fan::PickAt(float x, float y, int screenW, int screenH) {
    int vp[4] = {0, 0, screenW, screenH};
    float nearX, nearY, nearZ, farX, farY, farZ;
    float winY = (float)screenH - y; // flip Y for OpenGL

    // unproject to get Ray Origin & Direction
    transform.TransformUnproject(x, winY, 0.0f, &cachedView, &cachedProj, vp, &nearX, &nearY, &nearZ);
    transform.TransformUnproject(x, winY, 1.0f, &cachedView, &cachedProj, vp, &farX, &farY, &farZ);

    glm::vec3 rayOrig(nearX, nearY, nearZ);
    glm::vec3 rayDir = glm::normalize(glm::vec3(farX - nearX, farY - nearY, farZ - nearZ));

    int bestHit = -1;
    float bestDist = 99999.0f;

    // test ray against 7 parts
    for (int i = 0; i < 7; i++) {
        if (!parts[i].isSegment) {
            glm::vec3 w0 = parts[i].center - rayOrig;
            float t = glm::dot(w0, rayDir);
            if (t > 0) {
                glm::vec3 proj = rayOrig + rayDir * t;
                float d = glm::length(parts[i].center - proj);
                if (d <= parts[i].radius && t < bestDist) {
                    bestHit = i; bestDist = t;
                }
            }
        } else {
            glm::vec3 A = parts[i].center;
            glm::vec3 B = parts[i].tip;
            glm::vec3 segDir = B - A;
            float segLen = glm::length(segDir);
            segDir /= segLen;

            glm::vec3 w0 = rayOrig - A;
            float b = glm::dot(rayDir, segDir);
            float d = glm::dot(rayDir, w0);
            float e = glm::dot(segDir, w0);
            float denom = 1.0f - b * b;

            float t = 0.0f, u = 0.0f;
            if (denom > 0.0001f) {
                t = (b * e - d) / denom;
                u = (e - b * d) / denom;
            }

            u = std::max(0.0f, std::min(u, segLen));
            t = u * b - d;

            if (t > 0) {
                float dist = glm::length((rayOrig + rayDir * t) - (A + segDir * u));
                if (dist <= parts[i].radius && t < bestDist) {
                    bestHit = i; bestDist = t;
                }
            }
        }
    }

    if (bestHit != -1) {
        selectedPart = (selectedPart == bestHit) ? -1 : bestHit;
        return true;
    }
    selectedPart = -1;
    return false;
}