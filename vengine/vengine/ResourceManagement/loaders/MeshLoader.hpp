#pragma once

#include "../../graphics/Mesh.hpp"
#include "assimp/Importer.hpp" 
#include "assimp/mesh.h"
#include "assimp/scene.h"
#include "../ResourceManagerStructs.hpp" 
#include "TextureLoader.hpp"
class ResourceManager;

class MeshLoader
{   
private:
    Assimp::Importer     importer;
    VulkanImportStructs  importStructs;
    ResourceManager*     resourceMan;
    TextureLoader*       textureLoader = nullptr;
    A test;

    MeshData assimpImport(const std::string& modelFile);
    MeshData assimpMeshImport(const aiScene* scene,     std::vector<uint32_t>& materailToTexture);   

    std::vector<MeshData> getMeshesFromNodeTree(const aiScene * scene, const std::vector<uint32_t>& matToTex);
    MeshData loadMesh(aiMesh* mesh, uint32_t& lastVertice, uint32_t& lastIndex, std::vector<uint32_t> matToTex);

public: 
    void init(VmaAllocator*vma,vk::PhysicalDevice*physiscalDev,Device*dev,vk::Queue*transQueue,vk::CommandPool*transCmdPool, ResourceManager* resourceMan);
    void setTextureLoader(TextureLoader* textureLoader);

    Mesh createMesh(std::string path);
};