#pragma once

#include "../../graphics/Mesh.hpp"
#include "assimp/Importer.hpp" 
#include "assimp/mesh.h"
#include "assimp/scene.h"

#include "../ResourceManagerStructs.hpp"

class ResourceManager;
class MeshLoader
{   
private:
    static Assimp::Importer     importer;
    static VulkanImportStructs  importStructs;
    static ResourceManager*     resourceMan;

    static MeshData assimpImport(const std::string& modelFile);
    static MeshData assimpMeshImport(const aiScene* scene,     std::vector<uint32_t>& materailToTexture);   

    static std::vector<MeshData> getMeshesFromNodeTree(const aiScene * scene, const std::vector<uint32_t>& matToTex);
    static MeshData loadMesh(aiMesh* mesh, uint32_t& lastVertice, uint32_t& lastIndex, std::vector<uint32_t> matToTex);

public: 
    static void init(VmaAllocator*vma,vk::PhysicalDevice*physiscalDev,Device*dev,vk::Queue*transQueue,vk::CommandPool*transCmdPool, ResourceManager* resourceMan);

    static Mesh createMesh(std::string path);
};