// TextureSampling.metal - Metal texture sampling shader
// Based on Apple's Metal sample code

#include <metal_stdlib>
using namespace metal;

struct VertexOut {
    float4 position [[position]];
    float2 texCoord;
};

// Vertex shader with texture coordinates
vertex VertexOut textureVertexShader(uint vertexID [[vertex_id]],
                                     constant float4* positions [[buffer(0)]],
                                     constant float2* texCoords [[buffer(1)]]) {
    VertexOut out;
    out.position = positions[vertexID];
    out.texCoord = texCoords[vertexID];
    return out;
}

// Fragment shader that samples from a texture
fragment float4 textureFragmentShader(VertexOut in [[stage_in]],
                                      texture2d<float> texture [[texture(0)]],
                                      sampler textureSampler [[sampler(0)]]) {
    return texture.sample(textureSampler, in.texCoord);
}

// Fragment shader with texture filtering options
fragment float4 textureFilterFragmentShader(VertexOut in [[stage_in]],
                                            texture2d<float> texture [[texture(0)]],
                                            sampler textureSampler [[sampler(0)]]) {
    // Sample with linear filtering
    float4 color = texture.sample(textureSampler, in.texCoord);
    
    // Apply simple gamma correction
    color.rgb = pow(color.rgb, float3(2.2));
    
    return color;
}
