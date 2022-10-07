#pragma once
#include <unordered_map>

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
    ResourceManager*     resourceMan    = nullptr;
    TextureLoader*       textureLoader  = nullptr;

    MeshData assimpImport(const std::string& modelFile);
    MeshData assimpMeshImport(const aiScene* scene,     std::vector<uint32_t>& materialToTexture);   

    std::vector<MeshData> getMeshesFromNodeTree(const aiScene * scene, const std::vector<uint32_t>& matToTex);
    MeshData loadMesh(aiMesh* mesh, uint32_t& lastVertice, uint32_t& lastIndex, std::vector<uint32_t> matToTex);
    bool loadBones(const aiScene* scene, aiMesh* mesh, MeshData& outBoneData);

    aiNodeAnim* findAnimationNode(aiNodeAnim** nodeAnims, unsigned int numNodes, std::string_view name);
    aiNode* findNode(aiNode* rootNode, std::string_view boneName);
    aiNode* findParentBoneNode(std::unordered_map<std::string_view, int>& bones, aiNode* node);

public: 
    void init(VmaAllocator* vma, vk::PhysicalDevice* physiscalDev, Device* dev, vk::Queue* transQueue, vk::CommandPool* transCmdPool, ResourceManager* resourceMan);
    void setTextureLoader(TextureLoader* textureLoader);

    MeshData importMeshData(std::string& path);
    Mesh createMesh(MeshData& data);

};