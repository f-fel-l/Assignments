#include "Ground.h"
#include "ShaderHelper.h"

// quad (100x100) lying on XZ plane
// Layout: Position (x, y, z), Normal (nx, ny, nz)
static const GLfloat kGroundVertices[] = {
        -50.0f, 0.0f, -50.0f,   0.0f, 1.0f, 0.0f,
        50.0f, 0.0f, -50.0f,   0.0f, 1.0f, 0.0f,
        50.0f, 0.0f,  50.0f,   0.0f, 1.0f, 0.0f,
        -50.0f, 0.0f,  50.0f,   0.0f, 1.0f, 0.0f
};

static const GLuint kGroundIndices[] = {
        0, 1, 2,
        2, 3, 0
};

#ifdef PLATFORM_ANDROID
Ground::Ground(AAssetManager* assetMgr) : mgr(assetMgr) { modelType = TriangleType; }
#else
Ground::Ground() { modelType = TriangleType; }
#endif

Ground::~Ground() {
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (ibo) glDeleteBuffers(1, &ibo);
    if (program) glDeleteProgram(program);
}

void Ground::InitModel() {
    transform.TransformInit();

#ifdef PLATFORM_ANDROID
    program = ShaderHelper::buildProgramFromAssets(mgr, "shader/GroundVertex.glsl", "shader/GroundFragment.glsl");
#else
    program = ShaderHelper::buildProgramFromFile("GroundVertex.glsl", "GroundFragment.glsl");
#endif

    uMVP = glGetUniformLocation(program, "uMVP");
    uModel = glGetUniformLocation(program, "uModel");
    uNormalMatrix = glGetUniformLocation(program, "uNormalMatrix");
    uLightPos = glGetUniformLocation(program, "uLightPos");
    uViewPos = glGetUniformLocation(program, "uViewPos");

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kGroundVertices), kGroundVertices, GL_STATIC_DRAW);

    // Attribute 0: Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);

    // Attribute 1: Normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kGroundIndices), kGroundIndices, GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void Ground::Resize(int w, int h) {
    float aspect = (float)w / (float)h;
    transform.TransformSetMatrixMode(PROJECTION_MATRIX);
    transform.TransformLoadIdentity();
    transform.TransformSetPerspective(glm::radians(60.0f), aspect, 0.01f, 1000.0f, 0);
}

void Ground::Render() {
    if (!program || !vao) return;

    glUseProgram(program);
    glBindVertexArray(vao);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    transform.TransformSetMatrixMode(MODEL_MATRIX);
    transform.TransformLoadIdentity();

    // setup Camera in VIEW_MATRIX
    transform.TransformSetMatrixMode(VIEW_MATRIX);
    transform.TransformLoadIdentity();
    transform.TransformTranslate(0.0f, 0.8f, -cameraDistance);
    transform.TransformRotate(glm::radians(20.0f), 0.0f, 1.0f, 0.0f);

    // setup Ground in MODEL_MATRIX
    transform.TransformSetMatrixMode(MODEL_MATRIX);
    transform.TransformLoadIdentity();

    transform.TransformPushMatrix();
    transform.TransformTranslate(0.0f, -2.725f, 0.0f);

    // send matrices
    glUniformMatrix4fv(uMVP, 1, GL_FALSE, (const GLfloat*)transform.TransformGetModelViewProjectionMatrix());

    glm::mat4 model = *transform.TransformGetModelMatrix();
    glUniformMatrix4fv(uModel, 1, GL_FALSE, (const GLfloat*)&model[0][0]);

    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
    glUniformMatrix3fv(uNormalMatrix, 1, GL_FALSE, (const GLfloat*)&normalMatrix[0][0]);

    // send light data
    glUniform3f(uLightPos, 5.0f, 5.0f, 5.0f);
    glUniform3f(uViewPos, 0.0f, 0.0f, 0.0f);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    transform.TransformPopMatrix();
}