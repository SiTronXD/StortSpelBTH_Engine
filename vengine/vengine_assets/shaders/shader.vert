#version 460

#define FREQ_PER_FRAME 0
#define FREQ_PER_MESH 1

// Vertex data
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 nor;
layout(location = 2) in vec2 tex; 

// Uniform buffer
layout(set = FREQ_PER_FRAME, binding = 0) uniform CameraBuffer
{
    mat4 projection;
    mat4 view;
    vec4 worldPos;
} cameraBuffer;

// Push Constant to update the model Matrix! 
layout(push_constant) uniform PushConstant_Model
{
    mat4 model;
} pushConstant_Model;

// Vertex shader outputs
layout(location = 0) out vec3 fragWorldPos;
layout(location = 1) out vec3 fragNor;
layout(location = 2) out vec2 fragTex;
layout(location = 3) out vec3 fragCamWorldPos;

void main()
{
    vec4 worldPos = pushConstant_Model.model * vec4(pos, 1.0);

    gl_Position = 
        cameraBuffer.projection *
        cameraBuffer.view *
        worldPos;

    fragWorldPos = worldPos.xyz;
    fragNor = (pushConstant_Model.model * vec4(nor, 0.0f)).xyz;
    fragTex = tex;
    fragCamWorldPos = cameraBuffer.worldPos.xyz;
}