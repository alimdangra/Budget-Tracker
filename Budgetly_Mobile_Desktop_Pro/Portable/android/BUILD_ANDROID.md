# Android build instructions — shared C application

**Status:** Pure C application source and SDL2 integration files are provided.
There is no prebuilt APK in this project. You still need Android Studio,
Android SDK/NDK, and SDL2/SDL2_ttf Android libraries to package it.

1. Install Android Studio (Android SDK 34+ and NDK), Git, CMake support.
2. Get SDL2 **2.x**, and its official `android-project` starter from:
   https://github.com/libsdl-org/SDL (see `docs/README-android.md`).
3. Make a copy of SDL2's `android-project` folder, named `BudgetlyAndroid`.
   Place SDL2 sources in `BudgetlyAndroid/app/jni/SDL` and SDL2_ttf **2.x**
   sources in `BudgetlyAndroid/app/jni/SDL2_ttf`.
4. Copy `../src/Budgetly_Portable.c` and `../src/BudgetTracker_Core.inc`
   into `BudgetlyAndroid/app/jni/src/`. Copy `CMakeLists.txt` from this folder
   to `BudgetlyAndroid/app/jni/src/CMakeLists.txt`.
5. Use the CMake option in `BudgetlyAndroid/app/build.gradle`, pointing at its
   existing top-level JNI CMakeLists. If linking SDL2_ttf fails, configure its
   FreeType/Harfbuzz vendored dependencies according to SDL2_ttf's documentation.
6. Open `BudgetlyAndroid` in Android Studio, install dependencies, choose a
   connected Android device or emulator, then Build > Build APK.
7. Review the SDL2 template's `SDLActivity.java` and package name; it is
   **platform bootstrap** (required by Android), not your project logic.

The app's business logic/screens are pure C; Android needs Java/Kotlin
platform glue from the official SDL2 template because APKs require an Android
entrypoint. This cannot be avoided just by compiling a Windows `.exe`.

The app uses SDL_GetPrefPath for its own private account files and attempts
Android's Roboto system font. No account/password data is automatically shared
across devices. Keep existing data backed up.

Official docs: https://wiki.libsdl.org/SDL2/README-android
