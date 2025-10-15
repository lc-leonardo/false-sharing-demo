@echo off
echo === False Sharing Project Compilation Script ===
echo.

REM Check if gcc is available
where gcc >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo Error: GCC not found in PATH
    echo Please install MinGW-w64 or add GCC to your PATH
    echo Download from: https://www.mingw-w64.org/
    pause
    exit /b 1
)

echo Found GCC, proceeding with compilation...
echo.

REM Compile all programs
echo Compiling false_sharing_demo.c...
gcc -fopenmp -O2 -o false_sharing_demo.exe false_sharing_demo.c
if %ERRORLEVEL% NEQ 0 (
    echo Failed to compile false_sharing_demo.c
    pause
    exit /b 1
)
echo ✓ false_sharing_demo.exe created successfully

echo Compiling false_sharing_optimized.c...
gcc -fopenmp -O2 -o false_sharing_optimized.exe false_sharing_optimized.c
if %ERRORLEVEL% NEQ 0 (
    echo Failed to compile false_sharing_optimized.c
    pause
    exit /b 1
)
echo ✓ false_sharing_optimized.exe created successfully

echo Compiling false_sharing_comparison.c...
gcc -fopenmp -O2 -o false_sharing_comparison.exe false_sharing_comparison.c
if %ERRORLEVEL% NEQ 0 (
    echo Failed to compile false_sharing_comparison.c
    pause
    exit /b 1
)
echo ✓ false_sharing_comparison.exe created successfully

echo.
echo === Compilation Complete! ===
echo.
echo Available programs:
echo - false_sharing_demo.exe         (demonstrates the problem)
echo - false_sharing_optimized.exe    (shows the solution)
echo - false_sharing_comparison.exe   (compares both versions)
echo.
echo Run the comparison program for best results:
echo   false_sharing_comparison.exe
echo.
pause