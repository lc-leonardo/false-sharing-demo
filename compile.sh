#!/bin/bash

echo "=== False Sharing Project Compilation Script ==="
echo

# Check if gcc is available
if ! command -v gcc &> /dev/null; then
    echo "Error: GCC not found"
    echo "Please install GCC:"
    echo "  Ubuntu/Debian: sudo apt-get install gcc libomp-dev"
    echo "  CentOS/RHEL:   sudo yum install gcc libgomp-devel"
    echo "  macOS:         brew install gcc libomp"
    exit 1
fi

echo "Found GCC, proceeding with compilation..."
echo

# Compile all programs
echo "Compiling false_sharing_demo.c..."
if gcc -fopenmp -O2 -o false_sharing_demo false_sharing_demo.c; then
    echo "✓ false_sharing_demo created successfully"
else
    echo "Failed to compile false_sharing_demo.c"
    exit 1
fi

echo "Compiling false_sharing_optimized.c..."
if gcc -fopenmp -O2 -o false_sharing_optimized false_sharing_optimized.c; then
    echo "✓ false_sharing_optimized created successfully"
else
    echo "Failed to compile false_sharing_optimized.c"
    exit 1
fi

echo "Compiling false_sharing_comparison.c..."
if gcc -fopenmp -O2 -o false_sharing_comparison false_sharing_comparison.c; then
    echo "✓ false_sharing_comparison created successfully"
else
    echo "Failed to compile false_sharing_comparison.c"
    exit 1
fi

echo
echo "=== Compilation Complete! ==="
echo
echo "Available programs:"
echo "- ./false_sharing_demo         (demonstrates the problem)"
echo "- ./false_sharing_optimized    (shows the solution)"
echo "- ./false_sharing_comparison   (compares both versions)"
echo
echo "Run the comparison program for best results:"
echo "  ./false_sharing_comparison"
echo

# Make executables runnable
chmod +x false_sharing_demo false_sharing_optimized false_sharing_comparison

echo "Programs are ready to run!"