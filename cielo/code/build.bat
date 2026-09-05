@echo off

set COUNTER_FILE=build_count.txt
if not exist %COUNTER_FILE% echo 0 > %COUNTER_FILE%
set /p BUILD_COUNT=<%COUNTER_FILE%
set /a BUILD_COUNT+=1
echo %BUILD_COUNT% > %COUNTER_FILE%
echo BUILD #%BUILD_COUNT%
echo cwd: %CD%


set INCLUDES=/I.

set CommonCompilerFlags=/utf-8 /std:c17 /Zc:__STDC__ /EHsc ^
    /MD -nologo -fp:fast -Gm- -Od -Oi -WX -W4 ^
    -wd4202 -wd4100 -wd4189 -wd4244 -wd4996 -wd4456 -wd4324 -wd4505 -wd4267 -wd5287 -FC -Z7 ^
    -wd4701 ^
    /IC:\vcpkg\vcpkg-2026.07.29\installed\x64-windows\include ^
    %INCLUDES% 

set LDFLAGS=/LIBPATH:C:\vcpkg\vcpkg-2026.07.29\installed\x64-windows\lib ^
    /LIBPATH:..\Lib ^
    meshoptimizer.lib opengl32.lib user32.lib gdi32.lib


REM ── MAIN EXE ──────────────────────────────────────────────────────────────
    echo [MAIN] Compiling terrain with opengl...
    cl /c %CommonCompilerFlags% ^
        .\gl_load.c ^
        .\platform.c ^
        .\terrain_demo1.c ^
        .\terrain.c
    if %errorlevel% neq 0 ( echo [MAIN] compile failed & goto end )

    echo [MAIN] Linking terrain.exe...
    link /INCREMENTAL:NO /OPT:REF /DEBUG ^
        /PDB:terrain.pdb ^
        /out:terrain.exe ^
        gl_load.obj platform.obj terrain_demo1.obj terrain.obj ^
        %LDFLAGS%
    if %errorlevel% neq 0 ( echo [MAIN] link failed & goto end )

    echo [MAIN] terrain.exe built with OpenGL. 
)

:end
