#version 460

#extension GL_ARB_shader_viewport_layer_array : enable

#define FREQ_PER_FRAME 0
#define FREQ_PER_MESH 1
#define FREQ_PER_DRAW 2

// Vertex data
layout(location = 0) in vec3 pos;
layout(location = 1) in vec4 weights;
layout(location = 2) in uvec4 boneIndices;

// Storage buffer
struct BoneTransformsData
{
    mat4 boneTransform;
};
layout(std140, set = FREQ_PER_MESH, binding = 0) readonly buffer BoneTransformsBuffer
{
    BoneTransformsData transforms[];
} bones;

// Uniform buffer
#define MAX_NUM_CASCADES 4
layout(set = FREQ_PER_FRAME, binding = 0) uniform ShadowMapVP
{
    mat4 viewProjection[MAX_NUM_CASCADES];
} shadowMapVpBuffer;

// Push constant
layout(push_constant) uniform PushConstantData
{
    mat4 model;
} pushConstantData;

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

    gl_Position = 
        shadowMapVpBuffer.viewProjection[gl_InstanceIndex] *
        pushConstantData.model * 
        animTransform * 
        vec4(pos, 1.0);
        
    gl_Layer = gl_InstanceIndex;
}