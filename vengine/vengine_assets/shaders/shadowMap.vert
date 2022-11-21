#version 460

#define FREQ_PER_FRAME 0
#define FREQ_PER_MESH 1
#define FREQ_PER_DRAW 2

// Vertex data
layout(location = 0) in vec3 pos;

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