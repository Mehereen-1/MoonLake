@echo off
cd /d "%~dp0"
echo =======================================================
echo Building MoonLake (Debug)...
echo =======================================================
cmake --build build --config Debug
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] Build failed! Check the compiler errors above.
    pause
    exit /b %ERRORLEVEL%
)
echo.
echo =======================================================
echo Launching MoonLake 3D Scene...
echo =======================================================
build\Debug\MoonLake.exe
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo Application exited with code %ERRORLEVEL%.
    pause
)
