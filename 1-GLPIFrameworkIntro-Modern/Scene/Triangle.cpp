#define LOG_TAG "Triangle"
#include "Triangle.h"
#include "ShaderHelper.h"
#include <cmath>

// Clip-space triangle positions (same as original Cookbook values)
static const GLfloat kPositions[] = {
     0.0f,  0.5f,   // top
    -0.5f, -0.5f,   // bottom-left
     0.5f, -0.5f    // bottom-right
};

// Per-vertex colors: red, green, blue
static const GLfloat kColors[] = {
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f
};

static constexpr GLuint ATTRIB_POSITION = 0;
static constexpr GLuint ATTRIB_COLOR    = 1;

// ---------------------------------------------------------------------------
// Constructor / Destructor
// ---------------------------------------------------------------------------

#ifdef PLATFORM_ANDROID
Triangle::Triangle(AAssetManager* assetMgr)
    : mgr(assetMgr)
{
#else
Triangle::Triangle()
{
#endif
    modelType = TriangleType;
    degree    = 0.0f;
}

Triangle::~Triangle()
{
    if (vao)    { glDeleteVertexArrays(1, &vao);    vao    = 0; }
    if (vboPos) { glDeleteBuffers(1, &vboPos);      vboPos = 0; }
    if (vboColor){ glDeleteBuffers(1, &vboColor);   vboColor = 0; }
    if (program){ glDeleteProgram(program);          program = 0; }
}

// ---------------------------------------------------------------------------
// InitModel – compile shaders, create VAO/VBO, cache uniform location
// ---------------------------------------------------------------------------
void Triangle::InitModel()
{
    LOGI("Triangle::InitModel");

#ifdef PLATFORM_ANDROID
    program = ShaderHelper::buildProgramFromAssets(
        mgr,
        "shader/BlueTriangleVertex.glsl",
        "shader/BlueTriangleFragment.glsl");
#else
    program = ShaderHelper::buildProgramFromFile(
        "BlueTriangleVertex.glsl",
        "BlueTriangleFragment.glsl");
#endif

    if (!program) {
        LOGE("Triangle: failed to build shader program");
        return;
    }

    uRadianAngle = glGetUniformLocation(program, "RadianAngle");

    // --- VAO ---
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // --- Position VBO (static) ---
    glGenBuffers(1, &vboPos);
    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kPositions), kPositions, GL_STATIC_DRAW);
    glEnableVertexAttribArray(ATTRIB_POSITION);
    glVertexAttribPointer(ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    // --- Color VBO (static) ---
    glGenBuffers(1, &vboColor);
    glBindBuffer(GL_ARRAY_BUFFER, vboColor);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kColors), kColors, GL_STATIC_DRAW);
    glEnableVertexAttribArray(ATTRIB_COLOR);
    glVertexAttribPointer(ATTRIB_COLOR, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    LOGI("Triangle::InitModel done");
}

// ---------------------------------------------------------------------------
// Render – advance rotation angle and draw
// ---------------------------------------------------------------------------
void Triangle::Render()
{
    if (!program || !vao) return;

    glUseProgram(program);

    // Increment degree counter and convert to radians (matches original)
    float radian = degree++ / 57.2957795f;
    if (uRadianAngle >= 0)
        glUniform1f(uRadianAngle, radian);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}
