#version 460

#define FREQ_PER_MESH 1

// Storage buffer
struct UIElementData
{
    vec4 transform;
    vec4 multiplyColor;
    uvec4 uvRect;
};
layout(std140, set = FREQ_PER_MESH, binding = 0) readonly buffer UIElementDataBuffer
{
    UIElementData data[];
} ui;

// Output data
layout(location = 0) out vec2 fragUV;
layout(location = 1) out vec4 fragMultiplyColor;
layout(location = 2) out uvec4 fragBoundsUV;

void main()
{
    uint localVertexIndex = gl_VertexIndex % 6;

    // These calculations are done to avoid array indexing
    vec2 position = vec2(0.0f, 0.0f);
    position.x = localVertexIndex < 3 ? 
        float(int(localVertexIndex / 2) * 2 - 1) : // 0, 1, 2
        float(localVertexIndex % 2 == 1 ? 1 : -1); // 3, 4, 5
    position.y = localVertexIndex < 3 ?
        float(localVertexIndex % 2 == 0 ? 1 : -1) : // 0, 1, 2
        float(int((5 - localVertexIndex) / 2) * 2 - 1); // 3, 4, 5

    // Arrays to avoid indexing into:
    /*const vec2 positions[6] = vec2[](
        vec2(-1.0,  1.0),
        vec2(-1.0, -1.0),
        vec2( 1.0,  1.0),
        vec2( 1.0,  1.0),
        vec2(-1.0, -1.0),
        vec2( 1.0, -1.0)
    );
    vec2 uvs[6] = vec2[](
        vec2(0.0f, 0.0f),
        vec2(0.0f, 1.0f),
        vec2(1.0f, 0.0f),

        vec2(1.0f, 0.0f),
        vec2(0.0f, 1.0f),
        vec2(1.0f, 1.0f)
    );*/

    // Extract element data
    UIElementData uiData = ui.data[uint(gl_VertexIndex / 6)];
    vec4 uiTransform = uiData.transform;
    vec4 uvRect = vec4(uiData.uvRect);

    // Position
    gl_Position = vec4(
        uiTransform.xy + (position * 0.5f * uiTransform.zw), 
        0.5f, 
        1.0f
    );

    // UV coordinates
    vec2 localUV = vec2(position.x, -position.y) * 0.5f + vec2(0.5f);
    fragUV = uvRect.xy + localUV * uvRect.zw;
    fragBoundsUV = uiData.uvRect;

    // Multiply color
    fragMultiplyColor = uiData.multiplyColor;
}