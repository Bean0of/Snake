@echo off
mkdir build\win32
gcc -Wall -Wno-missing-braces -std=c99 -Iinclude -Llibs/win32 src/main.c -o build/win32/snake.exe -lraylib -lopengl32 -lwinmm -lgdi32 -DPLATFORM_DESKTOP && build\win32\snake.exe