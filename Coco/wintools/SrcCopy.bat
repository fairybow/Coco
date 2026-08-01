@echo off
setlocal

REM ----------------------------------------

set ROOT_DIR=%~dp0..\..
set PROJ_DIR=%~dp0..
set INCLUDE_DIR=%~dp0..\include\Coco
set SRC_DIR=%~dp0..\src
set DOCS_DIR=%~dp0..\docs

set OUT_DIR=%~dp0..\..\..\CocoSrc

REM ----------------------------------------

if exist "%OUT_DIR%" rmdir /s /q "%OUT_DIR%"
mkdir "%OUT_DIR%"

copy "%ROOT_DIR%\README.md" "%OUT_DIR%\" > nul

copy "%PROJ_DIR%\.clang-format" "%OUT_DIR%\" > nul
copy "%PROJ_DIR%\CMakeLists.txt" "%OUT_DIR%\" > nul
copy "%PROJ_DIR%\cpp.hint" "%OUT_DIR%\" > nul
copy "%PROJ_DIR%\webkit.clang-format.reference" "%OUT_DIR%\" > nul

for /r "%INCLUDE_DIR%" %%f in (*) do (
    copy "%%f" "%OUT_DIR%\" > nul
)

for /r "%SRC_DIR%" %%f in (*) do (
    copy "%%f" "%OUT_DIR%\" > nul
)

for /r "%DOCS_DIR%" %%f in (*) do (
    copy "%%f" "%OUT_DIR%\" > nul
)

REM ----------------------------------------

pushd "%ROOT_DIR%"
tree /f /a > "%OUT_DIR%\_tree.txt"
popd

pushd "%OUT_DIR%"
echo Copied to: %CD%
popd

pause
endlocal