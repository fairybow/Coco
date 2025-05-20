@echo off

:: Change directory to the Git repository root
for /f "delims=" %%i in ('git rev-parse --show-toplevel') do (
    cd "%%i"
    echo Changed to Git root: %%i
)

:: "delims=" option specifies that the delimiter is set to nothing, meaning the
:: entire output line will be treated as a single string

:: The loop will only run once, but AFAIK we need it to capture the output of
:: the the git command for a batch script
