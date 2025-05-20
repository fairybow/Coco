@echo off

:: Define variables for paths
set "BASE_DIR=C:\Dev\Coco\Coco"
set "SRC_DIR=.."

:: Define folders to process
set "FOLDERS=bookmarks copy include notes old scripts src"

:: Process each folder
for %%F in (%FOLDERS%) do (
    echo Processing %%F...
    rmdir /s /q "%BASE_DIR%\%%F"

    :: Add debug info
    echo Copying from: "%SRC_DIR%\%%F\" 
    echo To: "%BASE_DIR%\%%F\"
    
    xcopy /y /e /i "%SRC_DIR%\%%F\" "%BASE_DIR%\%%F\"
    
    :: Check if xcopy succeeded
    if errorlevel 1 (
        echo Error copying %%F - please check paths
    ) else (
        echo Successfully copied %%F
    )
)

echo Done!
pause
