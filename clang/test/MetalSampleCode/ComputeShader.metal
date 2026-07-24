// ComputeShader.metal - Basic Metal compute shader
// Based on Apple's Metal sample code

#include <metal_stdlib>
using namespace metal;

// Simple compute kernel that adds two arrays
kernel void addArrays(device const float* inA [[buffer(0)]],
                      device const float* inB [[buffer(1)]],
                      device float* result [[buffer(2)]],
                      uint index [[thread_position_in_grid]]) {
    result[index] = inA[index] + inB[index];
}

// Compute kernel that fills an array with a value
kernel void fillArray(device float* array [[buffer(0)]],
                      constant float& value [[buffer(1)]],
                      uint index [[thread_position_in_grid]]) {
    array[index] = value;
}

// Compute kernel that performs matrix multiplication
kernel void matrixMultiply(device const float* A [[buffer(0)]],
                           device const float* B [[buffer(1)]],
                           device float* C [[buffer(2)]],
                           constant uint& M [[buffer(3)]],
                           constant uint& N [[buffer(4)]],
                           constant uint& K [[buffer(5)]],
                           uint2 gid [[thread_position_in_grid]]) {
    uint row = gid.y;
    uint col = gid.x;
    
    if (row < M && col < N) {
        float sum = 0.0;
        for (uint k = 0; k < K; ++k) {
            sum += A[row * K + k] * B[k * N + col];
        }
        C[row * N + col] = sum;
    }
}
