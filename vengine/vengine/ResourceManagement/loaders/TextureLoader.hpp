#pragma once

#include "../ResourceManagerStructs.hpp"

typedef unsigned char stbi_uc;

/// TODO: REMOVE VulkanRenderer (Maybe?)
class VulkanRenderer;
class aiScene;
class ResourceManager;

#include <iostream>
class A {
public:
    void test() { std::cout << "hej\n"; }
};

class TextureLoader{

    
private:
    friend class MeshLoader;
    VulkanImportStructs  importStructs;
    ResourceManager*     resourceMan;

    std::vector<std::string> assimpGetTextures(const aiScene* scene);

    void      createTextureImage(const std::string &filename, vk::Image& ref, VmaAllocation& allocRef);
    uint32_t  createTextureDescriptor(vk::ImageView textureImage);
    stbi_uc*  loadTextureFile(const std::string &filename, int* width, int* height, vk::DeviceSize* imageSize);
public:
    VulkanRenderer* TEMP;        /// TODO: REMOVE VulkanRenderer
    void      assimpTextureImport(const aiScene* scene, std::vector<uint32_t>& materialToTexture);
    void      assimpTextureImport2();
    inline const void setVulkanRenderer(VulkanRenderer* ref) { this->TEMP = ref; };  /// TODO: REMOVE VulkanRenderer
    void init(VmaAllocator*vma,vk::PhysicalDevice*physiscalDev,Device*dev,vk::Queue*transQueue,vk::CommandPool*transCmdPool, ResourceManager*);
    ImageData createTexture(const std::string &filename);
    void cleanupTexture(ImageData& ref);
};