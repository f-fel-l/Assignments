# GLPI Framework Intro: Cross-Platform Renderer

This project is a cross-platform OpenGL ES rendering framework build for Android Programming Quiz 1. It demonstrates `Model` + `Renderer` architecture capable of rendering multiple shapes simultaneously across Desktop, Android, and Web environments using a single shared C++ codebase.

The core logic lives in `Scene/` directory and is completely from platform-specific windowing or initialization code:
* `Model.h`: An abstract base class defining the lifecycle (`InitModel`, `Render`, `Resize`) for all renderable shapes.
* `Renderer.h` / `Renderer.cpp`: A singleton controller that owns a collection of Models. It is the only component that knows about the concrete shapes (`Triangle` and `Square`).
* `ShaderHelper.h`: A utility for compiling and linking shaders, supporting both inline C-strings and external `.glsl` files loaded at runtime (including reading from APK assets via `AAssetManager` on Android).
* `Triangle` & `Square`: Concrete implementations of the `Model` interface. Each owns its own GPU resources (VAO, VBOs, Shader Program) and loads its shaders from files.

## BUILD AND RUN INSTRUCTIONS

Project targets 3 different platforms. Before building, ensure development environment is set up with CMake, Visual Studio (MSVC), Android Studio, and the Emscripten SDK.

## 1. PC (Desktop/Windows)
Desktop build uses CMake and GLFW
1. Open terminal/command prompt at root of the project
2. Run provided batch script by either:
  - double clicking on script_build_and_run.bat file
  - ```cmd script_build_and_run.bat
3. A window will open displaying the rotating triangle and static square
4. Press ESC to exit

## 2. Android
Android build uses Gradle, CMake, and JNI to bridge the Java lifecycle to the C++ Renderer.
1. Open Android Studio
2. Select Open and navigate to android/ folder inside project directory
3. Wait for gradle sync to complete
4. From top menu, select Build > Clean Project to ensure fresh build
5. Select target device/emulator and click the green Run(Play) button

## 3. WebGL (Emscripten)
Web build uses Emscripten to compile the C++ source into WebAssembly
1. Open a terminal and ensure Emscripten environemnt is active (eg. by running emsdk_env.bat)
2. Navigate to root of the project and run the build script:
  - build_web.bat
3. Once build succeeds, navigate to generated web/ directory and start a local web server by:
  - cd web
  - python -m http.server 8080
4. Open a web browser and navigate to http://localhost:8080/

## CLEANUP
To ensure clean builds when switching platforms/submitting project, manually delete build_desktop/ and web/ output directories.