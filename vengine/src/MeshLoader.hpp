#pragma once

#include "NewModel.hpp"
#include "assimp/Importer.hpp" 
#include "assimp/mesh.h"
#include "assimp/scene.h"

#include "ResourceManagerStructs.hpp"

class MeshLoader
{   
private:
    static Assimp::Importer     importer;
    static VulkanImportStructs  importStructs;
    static MeshData assimpImport(const std::string& modelFile);
    static MeshData assimpMeshImport(const aiScene* scene,     std::vector<uint32_t>& materailToTexture);     //TODO: Use reference?    

    //static std::vector<std::string> loadMaterials(const aiScene* scene);    
    static std::vector<MeshData> getMeshesFromNodeTree(const aiScene * scene, const std::vector<uint32_t>& matToTex);
    static MeshData loadMesh(aiMesh* mesh, uint32_t& lastVertice, uint32_t& lastIndex, std::vector<uint32_t> matToTex);

public: 
    static void setVulkanStructures(VmaAllocator*vma,vk::PhysicalDevice*physiscalDev,Device*dev,vk::Queue*transQueue,vk::CommandPool*transCmdPool);

    static NewModel createMesh(std::string path);
};