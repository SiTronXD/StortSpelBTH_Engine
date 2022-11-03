#version 460

#define FREQ_PER_FRAME 0
#define FREQ_PER_MESH 1

// Vertex data
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 nor;
layout(location = 2) in vec2 tex; 

// Uniform buffer
layout(set = FREQ_PER_FRAME, binding = 0) uniform UboViewProjection
{
    mat4 projection;
    mat4 view;    
} uboViewProjection;

// Storage buffer
struct LightBufferData
{
    vec4 position;
    vec4 color;
    vec4 padding0;
    vec4 padding1;
};
layout(std140, set = FREQ_PER_MESH, binding = 0) readonly buffer LightBuffer
{
    LightBufferData lights[];
} lightBuffer;

// Push Constant to update the model Matrix! 
layout(push_constant) uniform PushConstant_Model
{
    mat4 model;
} pushConstant_Model;

// Vertex shader outputs
layout(location = 0) out vec3 fragNor;
layout(location = 1) out vec2 fragTex;

void main()
{
    vec4 worldPos = pushConstant_Model.model * vec4(pos, 1.0);

    gl_Position = 
        uboViewProjection.projection *
        uboViewProjection.view *
        worldPos;

    //fragNor = (pushConstant_Model.model * vec4(nor, 0.0f)).xyz;
    fragTex = tex;

    fragNor = lightBuffer.lights[0].color.xyz * 
        1.0f / length(lightBuffer.lights[0].position.xyz - worldPos.xyz);
}