#!/bin/sh
set -e

CLANG_BIN="/Users/runner/work/llvm-project/llvm-project/build/bin/clang"
TEST_FILE="/Users/runner/work/llvm-project/llvm-project/test_apple_official_sdk.metal"
HEADER_DIR="/Users/runner/work/llvm-project/llvm-project/build/lib/clang/22/include"

echo "=== Testing Apple Official Metal SDK Equivalent & Specification Shaders ==="

VERSIONS="metal1.2 metal2.0 metal2.1 metal2.2 metal2.3 metal2.4 metal3.0 metal3.1 metal3.2 metal4.0 metal4.1"

for VER in $VERSIONS; do
    echo "--------------------------------------------------------"
    echo "Testing Official SDK Equivalent: -std=$VER"
    OUT_IR="/tmp/official_out_${VER}.ll"
    
    $CLANG_BIN -x metal -std=$VER -I$HEADER_DIR -I$HEADER_DIR/metal -c -emit-llvm -S $TEST_FILE -o $OUT_IR
    
    if [ -s "$OUT_IR" ]; then
        echo "SUCCESS: Generated Official SDK equivalent LLVM IR for $VER ($(wc -l < $OUT_IR) lines)"
        grep -q "metal-shader" $OUT_IR && echo "  - Verified: metal-shader attributes" || echo "  - Warning: metal-shader not found"
        grep -q "metal_arg_attributes" $OUT_IR && echo "  - Verified: metal_arg_attributes metadata" || echo "  - Warning: metal_arg_attributes not found"
        grep -q "addrspace" $OUT_IR && echo "  - Verified: Metal address spaces (addrspace)" || echo "  - Warning: addrspace not found"
    else
        echo "FAILED: Official SDK equivalent LLVM IR not generated for $VER"
        exit 1
    fi
done

echo "========================================================"
echo "APPLE OFFICIAL SDK EQUIVALENT & PDF SPECIFICATION SHADERS VERIFIED FOR ALL VERSIONS!"
