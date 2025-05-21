@echo off

call "WinConsumerGitRoot.bat"

echo Updating submodules...
git submodule update --init --recursive --remote
if %ERRORLEVEL% equ 0 (
    echo Successfully updated submodules.
) else (
    echo Failed to update submodules. Error code: %ERRORLEVEL%
    exit /b %ERRORLEVEL%
)

echo Done!
pause
