#!/usr/bin/env bash
set -e
cd "$(dirname "$0")"
echo "Installing SDL2 + SDL2_ttf if needed:"
echo "  pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-SDL2 mingw-w64-ucrt-x86_64-SDL2_ttf"
echo "Compile the shared cross-platform pure C interface..."
gcc -std=c11 -O2 src/Budgetly_Portable.c -o Budgetly_Portable.exe $(pkg-config --cflags --libs sdl2 SDL2_ttf) -lm
./Budgetly_Portable.exe
