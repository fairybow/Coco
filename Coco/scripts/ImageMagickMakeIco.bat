@echo off
setlocal enabledelayedexpansion

:: Get the SVG file from drag and drop
set svg=%1

:: Check if SVG file was provided
if "%svg%"=="" (
    echo Drag an SVG file onto this script
    pause
    exit /b
)

:: Check if file exists
if not exist "%svg%" (
    echo Error: File not found: %svg%
    pause
    exit /b
)

:: Get the base name without extension
for %%f in ("%svg%") do set baseName=%%~nf

:: Create PNG files for each size
set pngs=
for %%s in (16 32 48 64 128 256) do (
    set png=!baseName!-%%s.png
    echo Creating %%s x %%s PNG...
    magick "%svg%" -density 600 -transparent white -background transparent -alpha on -channel RGBA -colorspace sRGB -filter Lanczos -resize %%sx%%s -depth 8 -define png:compression-level=0 -define png:compression-filter=0 "!png!"
    set pngs=!pngs! "!png!"
)

:: Create ICO file from all PNGs
echo Creating ICO file...
magick !pngs! "%baseName%.ico"

echo Done! Created:
for %%s in (16 32 48 64 128 256) do echo   %baseName%-%%s.png
echo   %baseName%.ico

pause
