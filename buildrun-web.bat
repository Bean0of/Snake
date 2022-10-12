@echo off
mkdir build\web
emcc -Wall -Wno-missing-braces -std=c99 -Iinclude -Llibs/web src/main.c -o build/web/index.html -lraylib -s USE_GLFW=3 -s ASYNCIFY -DPLATFORM_WEB --shell-file src/shell.html && start http://localhost:8000/