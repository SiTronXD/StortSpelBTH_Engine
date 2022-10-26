#pragma once

#include "../ResourceManagerStructs.hpp"
#include "assimp/scene.h"
typedef unsigned char stbi_uc;

/// TODO: REMOVE VulkanRenderer (Maybe?)
class VulkanRenderer;
class ResourceManager;
class Texture;

class TextureLoader{
private:
    friend class MeshLoader;
    VulkanImportStructs  importStructs;
    ResourceManager*     resourceMan    = nullptr;

    std::vector<std::string> assimpGetTextures(const aiScene* scene);

    void processTextureName(std::string& filePathFromModel, const std::string& texturesFolderPath);
    void createTextureImage(const std::string &filename, vk::Image& ref, VmaAllocation& allocRef, uint32_t& outputWidth, uint32_t& outputHeight);
    stbi_uc* loadTextureFile(const std::string &filename, int* width, int* height, vk::DeviceSize* imageSize);
public:
    
    VulkanRenderer* TEMP;        /// TODO: REMOVE VulkanRenderer
    void assimpTextureImport(const aiScene* scene, const std::string& texturesFolderPath, std::vector<uint32_t>& materialToTexture);
    inline const void setVulkanRenderer(VulkanRenderer* ref) { this->TEMP = ref; };  /// TODO: REMOVE VulkanRenderer
    void init(VmaAllocator*vma,vk::PhysicalDevice*physiscalDev,Device*dev,vk::Queue*transQueue,vk::CommandPool*transCmdPool, ResourceManager*);
    Texture createTexture(const std::string &filename, const uint32_t& textureSamplerIndex);
    bool doesTextureExist(const std::string& filePath);
};