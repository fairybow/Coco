@echo off

:: Change directory to the Git repository root of the consuming project
for /f "delims=" %%i in ('git rev-parse --show-toplevel') do (
    :: Save the current path
    set "SUBMODULE_PATH=%%i"
)

:: Navigate up two directories (from Coco submodule to consuming project root).
:: This assumes Coco is always at depth 2: consuming-project/submodules/Coco
cd /d "%SUBMODULE_PATH%\..\..\"

for /f "delims=" %%i in ('git rev-parse --show-toplevel') do (
    cd "%%i"
    echo Changed to parent Git root: %%i
)

:: Old (WinGitRoot):

::@echo off

:: Change directory to the Git repository root
::for /f "delims=" %%i in ('git rev-parse --show-toplevel') do (
::    cd "%%i"
::    echo Changed to Git root: %%i
::)

:: "delims=" option specifies that the delimiter is set to nothing, meaning the
:: entire output line will be treated as a single string

:: The loop will only run once, but AFAIK we need it to capture the output of
:: the the git command for a batch script
