#version 460

#define FREQ_PER_FRAME 0
#define FREQ_PER_MESH 1
#define FREQ_PER_DRAW 2

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

// Push constant
layout(push_constant) uniform PushConstantData
{
    mat4 model;
    vec4 tintColor;     // vec4(tintColor.rgb, tintColorAlpha)
    vec4 emissionColor; // vec4(emission.rgb, emissionIntensity)
    vec4 settings;      // vec4(receiveShadows, fogStart, fogAbsorption, 0)
    vec4 tiling;        // vec4(offsetX, offsetY, scaleX, scaleY)
} pushConstantData;

// Vertex shader outputs
layout(location = 0) out vec3 fragWorldPos;
layout(location = 1) out vec3 fragViewPos;
layout(location = 2) out vec3 fragNor;
layout(location = 3) out vec4 fragTex;			// vec4(fragTex, fogStartDist, fogAbsorption)
layout(location = 4) out vec4 fragCamWorldPos;
layout(location = 5) out vec4 fragTintCol;
layout(location = 6) out vec4 fragEmissionCol;
layout(location = 7) out vec4 fragTiling;

void main()
{
    vec4 worldPos = pushConstantData.model * vec4(pos, 1.0);
    vec4 viewPos = cameraBuffer.view * worldPos;

    gl_Position = 
        cameraBuffer.projection *
        viewPos;

    fragWorldPos = worldPos.xyz;
    fragViewPos = viewPos.xyz;
    fragNor = (pushConstantData.model * vec4(nor, 0.0f)).xyz;
    fragTex = vec4(tex, pushConstantData.settings.yz);
    fragCamWorldPos = vec4(cameraBuffer.worldPos.xyz, pushConstantData.settings.x);
    fragTintCol = pushConstantData.tintColor;
    fragEmissionCol = pushConstantData.emissionColor;
    fragTiling = pushConstantData.tiling;
}