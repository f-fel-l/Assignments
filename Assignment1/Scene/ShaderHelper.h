#pragma once

/**
 * ShaderHelper.h
 *
 * Reusable shader compile/link helper shared by every Model. Supports two
 * ways of getting shader source into the same compile/link core: plain
 * strings (buildProgram, the "stringify" method) and files loaded from
 * disk / APK assets at runtime (buildProgramFromFile / buildProgramFromAssets).
 */

#include "Platform.h"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class ShaderHelper {
public:
    // --- Core compile/link, shared by both loading methods below
    static inline GLuint compileShader(GLenum type, const char* src) {
        GLuint shader = glCreateShader(type);
        if (!shader) { LOGE("glCreateShader failed (type=0x%x)", type); return 0; }
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);
        GLint ok = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            GLint len = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
            std::string log(static_cast<size_t>(len), '\0');
            glGetShaderInfoLog(shader, len, nullptr, &log[0]);
            LOGE("Shader compile error:\n%s", log.c_str());
            glDeleteShader(shader);
            return 0;
        }
        return shader;
    }

    static inline GLuint linkProgram(GLuint vert, GLuint frag) {
        GLuint prog = glCreateProgram();
        glAttachShader(prog, vert);
        glAttachShader(prog, frag);
        glLinkProgram(prog);

        GLint ok = GL_FALSE;
        glGetProgramiv(prog, GL_LINK_STATUS, &ok);
        if (!ok) {
            GLint len = 0;
            glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
            std::string log(static_cast<size_t>(len), '\0');
            glGetProgramInfoLog(prog, len, nullptr, &log[0]);
            LOGE("Program link error:\n%s", log.c_str());
            glDeleteProgram(prog);
            prog = 0;
        }
        glDetachShader(prog, vert);
        glDetachShader(prog, frag);
        glDeleteShader(vert);
        glDeleteShader(frag);
        return prog;
    }

    // --- Method 1: "stringify" - build directly from inline source strings
    static inline GLuint buildProgram(const char* vertSrc, const char* fragSrc) {
        GLuint vs = compileShader(GL_VERTEX_SHADER, vertSrc);
        if (!vs) return 0;
        GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragSrc);
        if (!fs) { glDeleteShader(vs); return 0; }
        return linkProgram(vs, fs);
    }

    // --- Method 2: file-based - load shader source from disk / APK assets
#ifdef PLATFORM_ANDROID
    static inline std::string loadAsset(AAssetManager* mgr, const char* path) {
        if (!mgr) return "";
        AAsset* asset = AAssetManager_open(mgr, path, AASSET_MODE_BUFFER);
        if (!asset) { LOGE("Failed to open asset: %s", path); return ""; }
        off_t length = AAsset_getLength(asset);
        std::string content(length, '\0');
        AAsset_read(asset, &content[0], length);
        AAsset_close(asset);
        return content;
    }

    static inline GLuint buildProgramFromAssets(AAssetManager* mgr, const char* vertPath, const char* fragPath) {
        std::string vSrc = loadAsset(mgr, vertPath);
        std::string fSrc = loadAsset(mgr, fragPath);
        if (vSrc.empty() || fSrc.empty()) return 0;
        return buildProgram(vSrc.c_str(), fSrc.c_str());
    }
#else
    static inline std::string loadFile(const char* filename) {
        std::string paths[] = {
                std::string("assets/shader/") + filename,
                std::string(filename),
                std::string("shader/") + filename,
                std::string("assets/") + filename
        };

        for (const auto& path : paths) {
            std::ifstream file(path.c_str());
            if (file.is_open()) {
                std::stringstream buffer;
                buffer << file.rdbuf();
                return buffer.str();
            }
        }
        LOGE("Failed to open file: %s", filename);
        return "";
    }

    static inline GLuint buildProgramFromFile(const char* vertFile, const char* fragFile) {
        std::string vSrc = loadFile(vertFile);
        std::string fSrc = loadFile(fragFile);
        if (vSrc.empty() || fSrc.empty()) return 0;
        return buildProgram(vSrc.c_str(), fSrc.c_str());
    }
#endif
};