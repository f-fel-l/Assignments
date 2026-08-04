# ===========================================================================
# ImportDependencies.cmake
#
# Downloads and configures third-party libraries needed for the desktop build.
# Android and Emscripten have their own GL implementations and do not use this.
#
# Dependencies:
#   GLFW  � window creation and input
#   GLEW  � OpenGL extension loader (static build)
# ===========================================================================

include(FetchContent)

function(importDependencies)

    if(ANDROID)
        # Android has GLESv3 built-in; nothing to fetch here.
        return()
    endif()

    # -----------------------------------------------------------------------
    # GLFW � window and input management
    # -----------------------------------------------------------------------
    message(STATUS "[ImportDependencies] Fetching GLFW ...")
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        3.5.1
        GIT_SHALLOW    TRUE
    )
    set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(glfw)

    # -----------------------------------------------------------------------
    # GLEW � OpenGL extension loader (static)
    # -----------------------------------------------------------------------
    message(STATUS "[ImportDependencies] Fetching GLEW ...")
    FetchContent_Declare(
        glew
        GIT_REPOSITORY https://github.com/Perlmint/glew-cmake.git
        GIT_TAG        glew-cmake-2.3.1
        GIT_SHALLOW    TRUE
    )
    set(glew-cmake_BUILD_SHARED OFF CACHE BOOL "" FORCE)
    set(glew-cmake_BUILD_STATIC ON  CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(glew)

    # -----------------------------------------------------------------------
    # GLM - header-only math library used by Transform.h/.cpp
    # -----------------------------------------------------------------------
    message(STATUS "[ImportDependencies] Fetching GLM ...")
    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG        0.9.9.8
    )
    # GLM 0.9.9.8's own CMakeLists predates CMake's minimum supported policy floor.
    set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
    FetchContent_MakeAvailable(glm)
    set(glm_SOURCE_DIR ${glm_SOURCE_DIR} PARENT_SCOPE)

    message(STATUS "[ImportDependencies] All desktop dependencies ready.")

endfunction()
