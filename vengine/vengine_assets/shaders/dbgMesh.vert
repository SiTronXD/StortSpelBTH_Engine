#version 460

#define FREQ_PER_FRAME 0

// Vertex data in world space
layout(location = 0) in vec3 pos;

// Uniform buffer
layout(set = FREQ_PER_FRAME, binding = 0) uniform UboViewProjection
{
    mat4 projection;
    mat4 view;    
} uboViewProjection;

// Push constant to update the model matrix
layout(push_constant) uniform PushConstantData
{
    mat4 model;
    vec4 color;
} pushConstantData;

// Output data
layout(location = 0) out vec3 fragCol;

void main()
{
    gl_Position = 
        uboViewProjection.projection *
        uboViewProjection.view *
        pushConstantData.model *
        vec4(pos, 1.0);

    fragCol = pushConstantData.color.rgb;
}