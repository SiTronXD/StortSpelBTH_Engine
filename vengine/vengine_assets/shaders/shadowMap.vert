#version 460

#extension GL_ARB_shader_viewport_layer_array : enable

#define FREQ_PER_FRAME 0
#define FREQ_PER_MESH 1
#define FREQ_PER_DRAW 2

// Vertex data
layout(location = 0) in vec3 pos;

// Uniform buffer
#define MAX_NUM_CASCADES 4
layout(set = FREQ_PER_FRAME, binding = 0) uniform ShadowMapVP
{
    mat4 viewProjection[MAX_NUM_CASCADES];
} shadowMapVpBuffer;

// Push constant
layout(push_constant) uniform PushConstantData
{
    mat4 model;
} pushConstantData;

void main()
{
    // Position
    gl_Position = 
        shadowMapVpBuffer.viewProjection[gl_InstanceIndex] *
        pushConstantData.model * 
        vec4(pos, 1.0);

    // Shadow map cascade layer
    gl_Layer = gl_InstanceIndex;
}