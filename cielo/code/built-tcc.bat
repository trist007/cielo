@echo off

set COUNTER_FILE=build_count.txt
if not exist %COUNTER_FILE% echo 0 > %COUNTER_FILE%
set /p BUILD_COUNT=<%COUNTER_FILE%
set /a BUILD_COUNT+=1
echo %BUILD_COUNT% > %COUNTER_FILE%
echo BUILD #%BUILD_COUNT%
echo cwd: %CD%


set INCLUDES=-I. -IGL
set LIBS= -lmeshoptimizer.lib -lopengl32.lib -luser32.lib -lgdi32.lib


REM ── MAIN EXE ──────────────────────────────────────────────────────────────
    echo [MAIN] Compiling terrain with opengl...
    tcc -c -g -Wall -O0 %INCLUDES% ^
        .\gl_load.c ^
        .\platform.c ^
        .\terrain_demo1.c ^
        .\terrain.c
    if %errorlevel% neq 0 ( echo [MAIN] compile failed & goto end )

    echo [MAIN] Linking terrain.exe...
    tcc -shared -o ^
        gl_load.o platform.o terrain_demo1.o terrain.o ^
        %LIBS%
    if %errorlevel% neq 0 ( echo [MAIN] link failed & goto end )

    echo [MAIN] terrain.exe built with OpenGL. 
)

:end
