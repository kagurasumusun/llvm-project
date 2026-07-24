// HelloTriangle.metal - Basic Metal triangle rendering shader
// Based on Apple's Metal sample code

#include <metal_stdlib>
using namespace metal;

// Vertex structure
struct Vertex {
    float4 position [[attribute(0)]];
    float4 color [[attribute(1)]];
};

// Vertex shader output
struct VertexOut {
    float4 position [[position]];
    float4 color;
};

// Vertex shader
vertex VertexOut vertexShader(uint vertexID [[vertex_id]],
                              constant Vertex* vertices [[buffer(0)]]) {
    VertexOut out;
    out.position = vertices[vertexID].position;
    out.color = vertices[vertexID].color;
    return out;
}

// Fragment shader
fragment float4 fragmentShader(VertexOut in [[stage_in]]) {
    return in.color;
}
