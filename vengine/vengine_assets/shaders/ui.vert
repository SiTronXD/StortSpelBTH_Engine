#version 460

#define FREQ_PER_MESH 1

// Storage buffer
struct UITransformsData
{
    vec4 uiTransform;
};
layout(std140, set = FREQ_PER_MESH, binding = 0) readonly buffer UITransformsBuffer
{
    UITransformsData transforms[];
} uiTransforms;

// Output data
layout(location = 0) out vec2 fragUV;

void main()
{
    const vec2 positions[6] = vec2[](
        vec2(-1.0,  1.0),
        vec2(-1.0, -1.0),
        vec2( 1.0,  1.0),
        vec2( 1.0,  1.0),
        vec2(-1.0, -1.0),
        vec2( 1.0, -1.0)
    );

    vec4 uiTran = 
        uiTransforms.transforms[uint(gl_VertexIndex / 6)].uiTransform;

    gl_Position = vec4(
        uiTran.xy + (positions[gl_VertexIndex % 6] * 0.5f * uiTran.zw), 
        0.5f, 
        1.0f
    );

    const vec2 uvs[6] = vec2[](
        vec2(0.0, 0.0),
        vec2(0.0, 1.0),
        vec2(1.0, 0.0),
        vec2(1.0, 0.0),
        vec2(0.0, 1.0),
        vec2(1.0, 1.0)
    );

    fragUV = uvs[gl_VertexIndex % 6];
}