#version 460

#define FREQ_PER_FRAME 0
#define FREQ_PER_MESH 1

// Vertex data
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 col;
layout(location = 2) in vec2 tex;
layout(location = 3) in vec4 weights;
layout(location = 4) in uvec4 boneIndices;

// Uniform buffer
layout(set = FREQ_PER_FRAME, binding = 0) uniform UboViewProjection
{
    mat4 projection;
    mat4 view;    
} uboViewProjection;

// Storage buffer
struct BoneTransformsData
{
    mat4 boneTransform;
};
layout(std140, set = FREQ_PER_MESH, binding = 0) readonly buffer BoneTransformsBuffer
{
    BoneTransformsData transforms[];
} bones;

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
        uboViewProjection.projection *
        uboViewProjection.view *
        pushConstant_Model.model *
        animTransform *
        vec4(pos, 1.0);

    fragCol = col;
    fragTex = tex;
}