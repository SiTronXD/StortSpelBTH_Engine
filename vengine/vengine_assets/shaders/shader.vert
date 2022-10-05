#version 460

#define FREQ_PER_FRAME 0
#define FREQ_PER_MESH 1

// Vertex data
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 col;
layout(location = 2) in vec2 tex; 

// Uniform buffer
layout(set = FREQ_PER_FRAME, binding = 0) uniform UboViewProjection
{
    mat4 projection;
    mat4 view;    
} uboViewProjection;

// Push Constant to update the model Matrix! 
layout(push_constant) uniform PushConstant_Model
{
    mat4 model;
} pushConstant_Model;

// Vertex shader outputs
layout(location = 0) out vec3 fragCol;
layout(location = 1) out vec2 fragTex;

void main()
{
    gl_Position = 
        uboViewProjection.projection *
        uboViewProjection.view *
        pushConstant_Model.model *
        vec4(pos, 1.0);

    fragCol = col;
    fragTex = tex;
}