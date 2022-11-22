#pragma once 
#include <string>
#include <vector>
#include <array>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>

struct ModelMatrix 
{
    glm::mat4 model; 
};

struct VertexStreams
{
    // "Default" meshes
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> colors;
    std::vector<glm::vec2> texCoords;

    // Animated meshes
    std::vector<glm::vec4> boneWeights;
    std::vector<glm::uvec4> boneIndices;
};

#define NUM_MAX_ANIMATION_SLOTS 5u // Defined here temp maybe we'll see
struct AnimationPlayer // find better name
{
    float timer = 0.f;
    float timeScale = 1.f;
    uint32_t animationIndex = 0u;
};

struct BonePoses
{
    std::vector<std::pair<float, glm::vec3>> translationStamps;
    std::vector<std::pair<float, glm::quat>> rotationStamps;
    std::vector<std::pair<float, glm::vec3>> scaleStamps;
};

struct Animation
{
    float endTime;
    std::vector<BonePoses> boneStamps;
};

struct Bone 
{
    std::string boneName;
    uint32_t slotIndex = 0u;
    int parentIndex;
    glm::mat4 inverseBindPoseMatrix;
    glm::mat4 boneMatrix;
};

struct SubmeshData
{
    uint32_t materialIndex;
    uint32_t startIndex;
    uint32_t numIndicies;
};

struct MeshData
{
    std::vector<SubmeshData> submeshes;
    VertexStreams vertexStreams;
    std::vector<uint32_t> indicies;
    std::vector<Bone> bones;
    std::vector<Animation> animations; 
};
