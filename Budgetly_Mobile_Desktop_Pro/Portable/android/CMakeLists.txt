# Android Studio's existing SDL2 android-project template supplies its Java
# bootstrap, AndroidManifest.xml, Gradle config, SDK and NDK integration.
# Place SDL2 under app/jni/SDL and SDL2_ttf under app/jni/SDL2_ttf.
cmake_minimum_required(VERSION 3.16)
project(BudgetlyMobile LANGUAGES C)
set(CMAKE_C_STANDARD 11)
set(SDL2TTF_VENDORED ON CACHE BOOL "Use bundled FreeType" FORCE)
add_subdirectory(../SDL SDL2-build)
add_subdirectory(../SDL2_ttf SDL2-ttf-build)
add_library(main SHARED Budgetly_Portable.c)
target_include_directories(main PRIVATE ../SDL/include ../SDL2_ttf)
target_link_libraries(main PRIVATE SDL2::SDL2 SDL2_ttf::SDL2_ttf android log m)
