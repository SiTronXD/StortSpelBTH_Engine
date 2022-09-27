#pragma once 
#include <vector>
#include <array>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

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
    std::vector<Vertex>  vertices;
    std::vector<uint32_t>    indicies;

};