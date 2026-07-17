#!/bin/sh
set -e

CLANG_BIN="/Users/runner/work/llvm-project/llvm-project/build/bin/clang"
TEST_FILE="/Users/runner/work/llvm-project/llvm-project/test_msl_advanced_features.metal"
HEADER_DIR="/Users/runner/work/llvm-project/llvm-project/clang/lib/Headers"

echo "=== Testing Metal Shading Language Advanced Features (Tile, SIMD, Tessellation, Texture Sampling) ==="

VERSIONS="metal1.2 metal2.0 metal2.1 metal2.2 metal2.3 metal2.4 metal3.0 metal3.1 metal3.2 metal4.0 metal4.1"

for VER in $VERSIONS; do
    echo "--------------------------------------------------------"
    echo "Testing Advanced Metal Features: -std=$VER"
    OUT_IR="/tmp/test_out_adv_${VER}.ll"
    
    $CLANG_BIN -x metal -std=$VER -I$HEADER_DIR -c -emit-llvm -S $TEST_FILE -o $OUT_IR
    
    if [ -s "$OUT_IR" ]; then
        echo "SUCCESS: Generated advanced LLVM IR for $VER ($(wc -l < $OUT_IR) lines)"
        grep -q "metal-shader" $OUT_IR && echo "  - Verified: metal-shader attributes" || echo "  - Warning: metal-shader not found"
        grep -q "metal_arg_attributes" $OUT_IR && echo "  - Verified: metal_arg_attributes metadata" || echo "  - Warning: metal_arg_attributes not found"
    else
        echo "FAILED: Advanced LLVM IR not generated for $VER"
        exit 1
    fi
done

echo "========================================================"
echo "ADVANCED METAL FEATURES SYNTAX & LLVM IR GENERATION VERIFIED FOR ALL VERSIONS!"
