#version 460

#define FREQ_PER_FRAME 0
#define FREQ_PER_MESH 1
#define FREQ_PER_DRAW 2

// Vertex data
layout(location = 0) in vec3 pos;

// Uniform buffer
layout(set = FREQ_PER_FRAME, binding = 0) uniform CameraBuffer
{
    mat4 projection;
    mat4 view;
    vec4 worldPos;
} cameraBuffer;

// Push constant
layout(push_constant) uniform PushConstantData
{
    mat4 model;
    vec4 tintColor;
} pushConstantData;

void main()
{
    vec4 worldPos = pushConstantData.model * vec4(pos, 1.0);

    gl_Position = 
        cameraBuffer.projection *
        cameraBuffer.view *
        worldPos;
}