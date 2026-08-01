@echo off
set CF="C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\Llvm\bin\clang-format.exe"

for /r "%~dp0..\include\Coco" %%f in (*.h *.mm) do (
    %CF% -i "%%f"
)

for /r "%~dp0..\src" %%f in (*.h *.cpp *.mm) do (
    %CF% -i "%%f"
)