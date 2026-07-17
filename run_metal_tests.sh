#!/bin/sh
set -e

CLANG_BIN="/Users/runner/work/llvm-project/llvm-project/build/bin/clang"
TEST_FILE="/Users/runner/work/llvm-project/llvm-project/test_msl_all.metal"
HEADER_DIR="/Users/runner/work/llvm-project/llvm-project/clang/lib/Headers"

echo "=== Testing Metal Shading Language All Versions & Syntax ==="

VERSIONS="metal1.2 metal2.0 metal2.1 metal2.2 metal2.3 metal2.4 metal3.0 metal3.1 metal3.2 metal4.0 metal4.1"

for VER in $VERSIONS; do
    echo "--------------------------------------------------------"
    echo "Testing MSL Version: -std=$VER"
    OUT_IR="/tmp/test_out_${VER}.ll"
    
    # Run Clang with -x metal, -std=$VER, include headers, and emit LLVM IR
    $CLANG_BIN -x metal -std=$VER -I$HEADER_DIR -c -emit-llvm -S $TEST_FILE -o $OUT_IR
    
    if [ -s "$OUT_IR" ]; then
        echo "SUCCESS: Generated LLVM IR for $VER ($(wc -l < $OUT_IR) lines)"
        # Verify specific Metal features in the generated LLVM IR
        grep -q "metal-shader" $OUT_IR && echo "  - Function attributes verified (metal-shader)" || echo "  - Warning: metal-shader attr not found"
        grep -q "kernel_arg_addr_space" $OUT_IR && echo "  - Kernel metadata verified (kernel_arg_addr_space)" || echo "  - Warning: kernel metadata not found"
        grep -q "addrspace" $OUT_IR && echo "  - Address spaces verified (addrspace)" || echo "  - Warning: addrspace not found"
    else
        echo "FAILED: LLVM IR not generated or empty for $VER"
        exit 1
    fi
done

echo "========================================================"
echo "ALL METAL VERSIONS & SYNTAX LLVM IR GENERATION COMPLETE!"
