:: I wrote a shell script when working with MacOS and then asked Gemini to convert it to a .bat script for windows
@echo off
setlocal enabledelayedexpansion

set "BLENDER_EXE=C:\Program Files\Blender Foundation\Blender 5.0\blender.exe"
set "PYTHON_SCRIPT=C:\Users\vivek\CLionProjects\raytracer2\scripts\create_scene.py"
set "RENDERER=C:\Users\vivek\CLionProjects\raytracer2\cmake-build-debug\apps\render_to_ppm.exe"

for /f "usebackq tokens=3" %%A in (`^""%BLENDER_EXE%" --background --python "%PYTHON_SCRIPT%" 2^>^&1 ^| findstr /C:"OBJ file:"^"`) do (
    set "OBJ_PATH=%%A"
)

if "%OBJ_PATH%"=="" (
    echo [ERROR] Failed to extract OBJ path
    pause
    exit /b 1
)

echo [INFO] Rendering: %OBJ_PATH%
"%RENDERER%" "%OBJ_PATH%"

set "PPM_FILE=%OBJ_PATH:.obj=.ppm%"
set "PNG_FILE=%OBJ_PATH:.obj=.png%"

if exist "%PPM_FILE%" (
    echo [INFO] Converting %PNG_FILE% to %PNG_FILE%...
    ffmpeg -y -i "%PPM_FILE%" "%PNG_FILE%" >nul 2>&1

    if !errorlevel! equ 0 (
        echo [SUCCESS] Image converted to PNG.
        del "%PPM_FILE%"
    ) else (
        echo [ERROR] FFmpeg conversion failed. Check if ffmpeg is in your PATH.
    )
) else (
    echo [ERROR] PPM file not found. Renderer may have failed.
)

pause