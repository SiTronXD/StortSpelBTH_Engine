#version 460

#define FREQ_PER_MESH 1

// Storage buffer
struct UIElementData
{
    vec4 subTextureRect;
    vec4 transform;
};
layout(std140, set = FREQ_PER_MESH, binding = 0) readonly buffer UIElementDataBuffer
{
    UIElementData data[];
} ui;

// Output data
layout(location = 0) out vec2 fragUV;

void main()
{
    // Extract element data
    UIElementData uiData = ui.data[uint(gl_VertexIndex / 6)];
    vec4 uiSubTextureRect = uiData.subTextureRect;
    vec4 uiTransform = uiData.transform;

    // Position
    const vec2 positions[6] = vec2[](
        vec2(-1.0,  1.0),
        vec2(-1.0, -1.0),
        vec2( 1.0,  1.0),
        vec2( 1.0,  1.0),
        vec2(-1.0, -1.0),
        vec2( 1.0, -1.0)
    );
    gl_Position = vec4(
        uiTransform.xy + (positions[gl_VertexIndex % 6] * 0.5f * uiTransform.zw), 
        0.5f, 
        1.0f
    );

    // Uv coordinates
    vec2 uvs[6] = vec2[](
        vec2(uiSubTextureRect.x,                        uiSubTextureRect.y),
        vec2(uiSubTextureRect.x,                        uiSubTextureRect.y + uiSubTextureRect.w),
        vec2(uiSubTextureRect.x + uiSubTextureRect.z,   uiSubTextureRect.y),
        vec2(uiSubTextureRect.x + uiSubTextureRect.z,   uiSubTextureRect.y),
        vec2(uiSubTextureRect.x,                        uiSubTextureRect.y + uiSubTextureRect.w),
        vec2(uiSubTextureRect.x + uiSubTextureRect.z,   uiSubTextureRect.y + uiSubTextureRect.w)
    );
    fragUV = uvs[gl_VertexIndex % 6];
}