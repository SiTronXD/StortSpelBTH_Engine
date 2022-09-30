#pragma once 
#include <vector>
#include <array>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>

struct ModelMatrix { 
    glm::mat4 model; 
};

struct Vertex
{
    glm::vec3 pos;      /// Vertex Position (x,y,z)
    glm::vec3 col;      /// Vertex Color    (r,g,b)
    //glm::vec3 nor;      /// texture normal (x,y,z)      //TODO: Add?
    glm::vec2 tex;      /// texture coords  (u,v)
};

struct AnimVertex 
{
    glm::vec3 pos;  /// Vertex Position (x,y,z)
    glm::vec3 col;  /// Vertex Color    (r,g,b)
    //glm::vec3 nor;      /// texture normal (x,y,z)      //TODO: Add?
    glm::vec2 tex;  /// texture coords  (u,v)
    float weights[4] { -1.f, -1.f, -1.f, -1.f };
    unsigned int bonesIndex[4] {};
};

struct Bone 
{
    int parentIndex;
    glm::mat4 inverseBindPoseMatrix;
    glm::mat4 modelMatrix;
    glm::mat4 finalMatrix;
    std::vector<std::pair<float, glm::vec3>> translationStamps;
    std::vector<std::pair<float, glm::quat>> rotationStamps; // quaternion x, y, z, w
    std::vector<std::pair<float, glm::vec3>> scaleStamps;
};

//TODO: BoneTranformations
//TODO: Bone
//TODO: AnimVertex
//TODO: DefaultMesh (??)

struct SubmeshData{
    uint32_t materialIndex;
    uint32_t startIndex;
    uint32_t numIndicies;
};

struct MeshData{
    std::vector<SubmeshData> submeshes;
    std::vector<Vertex> vertices;
    std::vector<AnimVertex> aniVertices;
    std::vector<uint32_t>    indicies;
    std::vector<Bone>        bones;
};
