#pragma once 
#include <vector>
#include <array>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

// TODO: Give this a  better name... Bad game given until legacy code is replaced...

const uint32_t MAX_SUBMESH_MATERIAL_NAME_LENGTH = 64;

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

struct SubMeshData{
    //std::array<char, MAX_SUBMESH_MATERIAL_NAME_LENGTH> materialName;
    uint32_t materialIndex;
    uint32_t startIndex;
    uint32_t numIndicies;
};

struct MeshData{
    std::vector<SubMeshData> submeshes;
    std::vector<Vertex>  vertices;
    std::vector<uint32_t>    indicies;

};