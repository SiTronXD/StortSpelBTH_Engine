#pragma once 
 #include "op_overload.hpp"
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

    MeshData assimpImport(const std::string& modelFile, const std::string& texturesFolderPath);
    MeshData assimpMeshImport(const aiScene* scene,     std::vector<uint32_t>& materialToTexture);   

    template <typename T>
    void insertStream(std::vector<T>& inStream, std::vector<T>& outputStream);
    void topologicallySortBones(aiMesh* mesh, aiNode* node, uint32_t& globalIndex);
    std::vector<MeshData> getMeshesFromNodeTree(const aiScene * scene, const std::vector<uint32_t>& matToTex);
    MeshData loadMesh(aiMesh* mesh, uint32_t& lastVertice, uint32_t& lastIndex, std::vector<uint32_t> matToTex);

    aiNodeAnim* findAnimationNode(aiNodeAnim** nodeAnims, unsigned int numNodes, std::string_view name);
    aiNode* findNode(aiNode* rootNode, std::string_view boneName);
    aiNode* findParentBoneNode(std::unordered_map<std::string_view, int>& bones, aiNode* node);

    // Animations
    void loadAnimation(const aiScene* scene, MeshData& outMeshData);
    void loadSkeleton(const aiScene* scene, MeshData& outMeshData);
public: 
    void init(
        VmaAllocator* vma, 
        PhysicalDevice* physicalDevice, 
        Device* dev, 
        vk::Queue* transQueue, 
        vk::CommandPool* transCmdPool, 
        ResourceManager* resourceMan);
    void setTextureLoader(TextureLoader* textureLoader);

    MeshData importMeshData(
        const std::string& modelPath,
        const std::string& texturesFolderPath);
    Mesh createMesh(MeshData& data);

    void loadAnimations(const std::vector<std::string>& paths,const std::string& textures, MeshData& outMeshData);
};