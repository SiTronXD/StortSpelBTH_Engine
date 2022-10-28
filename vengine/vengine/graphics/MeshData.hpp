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

struct Bone 
{
#if defined(_DEBUG) || defined(DEBUG)
    std::string boneName;
#endif

    int parentIndex;
    glm::mat4 inverseBindPoseMatrix;
    glm::mat4 boneMatrix;
    std::vector<std::pair<float, glm::vec3>> translationStamps;
    std::vector<std::pair<float, glm::quat>> rotationStamps; // quaternion x, y, z, w
    std::vector<std::pair<float, glm::vec3>> scaleStamps;
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
};
