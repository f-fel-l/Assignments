# Assignment 3 — 3D Fan

A 3D table fan — base, pole, hub, and four spinning blades — built from a
single 8-vertex cube drawn seven times with a different model matrix per part
on the `Transform` push/pop matrix stack, each part tinted through the
`PARTCOLOR` uniform. One shared C++ code base (`Scene/`) runs on **Desktop**
(Windows, GLFW + GLEW), **Android** (NDK), and **WebGL** (Emscripten).

Gestures (mouse on Desktop/Web = touch on Android):
- **Tap / click** — toggles the fan ON/OFF and logs `Fan ON` / `Fan OFF`.
- **Swipe / click-drag** — spins the fan faster proportional to drag speed;
  releasing returns it to base speed.

### Bonus features

- **Smooth zoom/speed easing** — instead of an instant jump per button press,
  the camera distance and fan speed ease toward their target values over a
  few frames rather than snapping immediately.
- **Press-and-hold** — holding down `+SPD` / `-SPD` / `+ZOOM` / `-ZOOM`
  repeats the action every frame instead of requiring repeated clicks, the
  same level-triggered pattern the Scene Graph HUD examples use for their
  D-pad.

## Prerequisites

| Target  | You need |
|---------|----------|
| Desktop | CMake 3.15+, Visual Studio 2022 ("Desktop development with C++" workload), internet access (GLFW/GLEW/GLM are fetched automatically by CMake) |
| Android | Android Studio, JDK 17, Android SDK with **NDK 27.0.12077973** and **CMake 3.22.1** (install both via SDK Manager → SDK Tools) |
| Web     | [Emscripten SDK](https://emscripten.org/docs/getting_started/) 3.1.64+ activated, Git (used to fetch GLM), Python 3 (to serve the output) |

## Build & Run — Desktop (Windows)

From the project root:

```
script_build_and_run.bat
```

This configures the project with CMake (Visual Studio 17 2022, x64) into
`build_desktop/`, builds the `Debug` configuration, and launches the exe. The
first configure downloads GLFW, GLEW, and GLM via `FetchContent` — allow a
few minutes.

Output: `build_desktop/Debug/GLPIFrameworkIntro.exe`
Controls: click = tap, click-drag = swipe, `ESC` quits.

Equivalent manual commands:

```
cmake -S . -B build_desktop -G "Visual Studio 17 2022" -A x64
cmake --build build_desktop --config Debug
build_desktop\Debug\GLPIFrameworkIntro.exe
```

## Build & Run — Android

1. Open the `android/` folder in Android Studio and let it sync (it will
   write `local.properties` with your SDK path).
2. Confirm NDK 27.0.12077973 and CMake 3.22.1 are installed (SDK Manager →
   SDK Tools).
3. Run ▶ on a device or emulator from Android Studio.

Command-line alternative (the Gradle wrapper is not checked into the repo —
use a system-installed **Gradle 8.13**):

```
cd android
gradle assembleDebug
adb install -r app/build/outputs/apk/debug/app-debug.apk
```

Output: `android/app/build/outputs/apk/debug/app-debug.apk`
Verify gestures: `adb logcat -s Fan` shows `Fan ON` / `Fan OFF` on tap and the
measured drag velocity on swipe.

## Build & Run — Web (Emscripten)

1. Activate the Emscripten SDK in your shell:
   `source /path/to/emsdk/emsdk_env.sh` (macOS/Linux) or
   `emsdk_env.bat` (Windows).
2. From the project root:

   ```
   bash build_web.sh      # or build_web.bat on Windows
   ```

   The first run clones GLM 0.9.9.8 into a local `.glm_cache/` folder (not
   committed) before invoking `emcc`.
3. Output: `web/index.html` (+ `index.js`, `index.wasm`, `index.data`).
4. Serve it locally and open in a browser:

   ```
   cd web && python -m http.server 8080
   ```

   Then visit `http://localhost:8080/`.

## Live demo

The `web` build is deployed automatically to GitHub Pages by
`.github/workflows/pages.yml` on every push to `main`.
