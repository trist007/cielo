@echo off

echo Compilation started at %date% %time%
echo,
set START_TIME=%time%
set START_TIME=%START_TIME: =0%
set COUNTER_FILE=build_count.txt
if not exist %COUNTER_FILE% echo 0 > %COUNTER_FILE%
set /p BUILD_COUNT=<%COUNTER_FILE%
set /a BUILD_COUNT+=1
echo %BUILD_COUNT% > %COUNTER_FILE%
echo BUILD #%BUILD_COUNT%
echo cwd: %CD%

set CPPFLAGS=/I. /I..\Include /I..\Common\FreetypeGL /I..\Common\3rdparty\ImGui\GLFW\ ^
    /I..\Include\assimp5\ ^
    /IC:\vcpkg\vcpkg-2026.07.29\installed\x64-windows\include ^
    /DFREEGLUT_LIB_PRAGMAS=0 ^
    /std:c++20 /EHsc /Zi

set LDFLAGS=/LIBPATH:C:\vcpkg\vcpkg-2026.07.29\installed\x64-windows\lib ^
    /LIBPATH:..\Lib ^
    glew32.lib assimp-vc143-mt.lib glfw3dll.lib meshoptimizer.lib freeglut.lib opengl32.lib user32.lib


REM ── MAIN EXE ──────────────────────────────────────────────────────────────
    echo [MAIN] Compiling terrain with glfw and opengl...
    cl /c %CPPFLAGS% ^
        .\terrain_demo1.cpp ^
        .\terrain.cpp ^
        .\triangle_list.cpp ^
        .\terrain_technique.cpp ^
        ..\Common\ogldev_util.cpp ^
        ..\Common\math_3d.cpp ^
        ..\Common\ogldev_basic_glfw_camera.cpp ^
        ..\Common\ogldev_glfw.cpp ^
        ..\Common\technique.cpp
    if %errorlevel% neq 0 ( echo [MAIN] compile failed & goto end )

    echo [MAIN] Linking terrain.exe...
    link /INCREMENTAL:NO /OPT:REF /DEBUG ^
        /PDB:terrain.pdb ^
        /out:terrain.exe ^
        terrain_demo1.obj terrain.obj triangle_list.obj terrain_technique.obj ^
        ogldev_util.obj math_3d.obj ogldev_basic_glfw_camera.obj ogldev_glfw.obj ^
        technique.obj ^
        %LDFLAGS%
    if %errorlevel% neq 0 ( echo [MAIN] link failed & goto end )

    echo [MAIN] terrain.exe built with OpenGL and GLFW.
)

REM ── TIMING ────────────────────────────────────────────────────────────────
:end
set END_TIME=%time%
set END_TIME=%END_TIME: =0%
set START_H=%START_TIME:~0,2%
set START_M=%START_TIME:~3,2%
set START_S=%START_TIME:~6,2%
set START_MS=%START_TIME:~9,2%
set END_H=%END_TIME:~0,2%
set END_M=%END_TIME:~3,2%
set END_S=%END_TIME:~6,2%
set END_MS=%END_TIME:~9,2%
set /a START_TOTAL=(((START_H*60)+START_M)*60)+START_S
set /a END_TOTAL=(((END_H*60)+END_M)*60)+END_S
set /a TOTAL_SEC=%END_TOTAL% - %START_TOTAL%
if %TOTAL_SEC% LSS 0 set /a TOTAL_SEC+=86400
set /a TOTAL_MS=%END_MS% - %START_MS%
if %TOTAL_MS% LSS 0 (
    set /a TOTAL_MS+=1000
    set /a TOTAL_SEC-=1
)
if %TOTAL_MS% LSS 10 set TOTAL_MS=0%TOTAL_MS%
echo.
if %errorlevel% equ 0 (
    echo Compilation finished at %date% %time%
) else (
    echo Compilation failed with errors at %date% %time%
)
echo ================================================
echo Build #%BUILD_COUNT% completed in %TOTAL_SEC%.%TOTAL_MS%s
