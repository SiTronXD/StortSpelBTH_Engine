#version 460

#define FREQ_PER_FRAME 0
#define FREQ_PER_MESH 1
#define FREQ_PER_DRAW 2

// Vertex data
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 nor;
layout(location = 2) in vec2 tex;
layout(location = 3) in vec4 weights;
layout(location = 4) in uvec4 boneIndices;

// Uniform buffer
layout(set = FREQ_PER_FRAME, binding = 0) uniform CameraBuffer
{
    mat4 projection;
    mat4 view;
    vec4 worldPos;
} cameraBuffer;

// Storage buffer
struct BoneTransformsData
{
    mat4 boneTransform;
};
layout(std140, set = FREQ_PER_MESH, binding = 0) readonly buffer BoneTransformsBuffer
{
    BoneTransformsData transforms[];
} bones;

// Push constant
layout(push_constant) uniform PushConstantData
{
    mat4 model;
    vec4 tintColor;
    vec4 emissionColor; // vec4(emission.rgb, receiveShadows)
} pushConstantData;

// Vertex shader outputs
layout(location = 0) out vec3 fragWorldPos;
layout(location = 1) out vec3 fragViewPos;
layout(location = 2) out vec3 fragNor;
layout(location = 3) out vec2 fragTex;
layout(location = 4) out vec3 fragCamWorldPos;
layout(location = 5) out vec4 fragTintCol;
layout(location = 6) out vec4 fragEmissionCol;

void main()
{
    mat4 animTransform = mat4(0.0f);
    if(weights.x > 0.0f)
        animTransform += bones.transforms[boneIndices.x].boneTransform * weights.x;
    if(weights.y > 0.0f)
        animTransform += bones.transforms[boneIndices.y].boneTransform * weights.y;
    if(weights.z > 0.0f)
        animTransform += bones.transforms[boneIndices.z].boneTransform * weights.z;
    if(weights.w > 0.0f)
        animTransform += bones.transforms[boneIndices.w].boneTransform * weights.w;

    vec4 worldPos = 
        pushConstantData.model *
        animTransform *
        vec4(pos, 1.0);

    vec4 viewPos = 
        cameraBuffer.view *
        worldPos;

    gl_Position = 
        cameraBuffer.projection *
        viewPos;
    
    fragWorldPos = worldPos.xyz;
    fragViewPos = viewPos.xyz;
    fragNor = (pushConstantData.model * animTransform * vec4(nor, 0.0f)).xyz;
    fragTex = tex;
    fragCamWorldPos = cameraBuffer.worldPos.xyz;
    fragTintCol = pushConstantData.tintColor;
    fragEmissionCol = pushConstantData.emissionColor;
}