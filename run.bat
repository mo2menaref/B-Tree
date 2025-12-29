@echo off
echo Compiling C code...
gcc bplus_tree.c -o bplus_tree.exe
if %errorlevel% neq 0 (
    echo Compilation Failed!
    pause
    exit /b
)
echo Starting Python Server...
python start_server.py