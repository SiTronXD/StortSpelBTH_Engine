#version 460

#define FREQ_PER_FRAME 0
#define FREQ_PER_MESH 1
#define FREQ_PER_DRAW 2

// Vertex data
layout(location = 0) in vec3 pos;

// Uniform buffer
layout(set = FREQ_PER_FRAME, binding = 0) uniform ShadowMapBuffer
{
    mat4 projection[4];
    mat4 view[4];
    vec2 shadowMapSize;
	float shadowMapMinBias;
	float shadowMapAngleBias;
} shadowMapBuffer;

// Push constant
layout(push_constant) uniform PushConstantData
{
    mat4 viewProjection;
    mat4 model;
} pushConstantData;

void main()
{
    gl_Position = 
        pushConstantData.viewProjection *
        pushConstantData.model * 
        vec4(pos, 1.0);
}