#version 460

#define FREQ_PER_FRAME 0
#define FREQ_PER_MESH 1
#define FREQ_PER_DRAW 2

// Vertex data
layout(location = 0) in vec3 pos;

// Uniform buffer
layout(set = FREQ_PER_FRAME, binding = 0) uniform ShadowMapBuffer
{
    mat4 projection;
    mat4 view;
    vec2 shadowMapSize;
	float shadowMapMinBias;
	float shadowMapAngleBias;
} shadowMapBuffer;

// Push constant
layout(push_constant) uniform PushConstantData
{
    mat4 model;
    vec4 tintColor;
} pushConstantData;

void main()
{
    gl_Position = 
        shadowMapBuffer.projection *
        shadowMapBuffer.view *
        pushConstantData.model * 
        vec4(pos, 1.0);
}