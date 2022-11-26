#pragma once

#include "../ResourceManagerStructs.hpp"
#include "assimp/scene.h"

class ResourceManager;
class Texture;

struct TextureSettings;

class TextureLoader
{
private:
    friend class MeshLoader;
    VulkanImportStructs  importStructs;
    ResourceManager*     resourceMan    = nullptr;

    void assimpGetTextures(const aiScene* scene, std::vector<std::string> &diffuseTextures, std::vector<std::string> &emissiveTextures);

    void processTextureName(std::string& filePathFromModel, const std::string& texturesFolderPath);
    stbi_uc* loadTextureFile(const std::string &filename, int* width, int* height);

public:
    void assimpTextureImport(const aiScene* scene, const std::string& texturesFolderPath, std::vector<uint32_t>& materialToTexture);
    void init(VmaAllocator* vma, PhysicalDevice* physiscalDev, Device* dev, vk::Queue* transQueue, vk::CommandPool* transCmdPool, ResourceManager* resourceMan);
    Texture createTexture(const std::string &filename, const TextureSettings& textureSettings, const uint32_t& textureSamplerIndex);
    bool doesTextureExist(const std::string& filePath);
};