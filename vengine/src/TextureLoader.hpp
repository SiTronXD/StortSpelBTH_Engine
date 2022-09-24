#pragma once

#include "ResourceManagerStructs.hpp"

typedef unsigned char stbi_uc;
/// TODO: REMOVE VulkanRenderer (Maybe?)
class VulkanRenderer;

class ResourceManager;
class TextureLoader{
public: 
    static VulkanRenderer*       TEMP;        /// TODO: REMOVE VulkanRenderer
    static ResourceManager*      resourceMan;
private:
    friend class MeshLoader;
    static VulkanImportStructs  importStructs;

    static std::vector<std::string> assimpGetTextures(const aiScene* scene);
    static void assimpTextureImport(const aiScene* scene, std::vector<uint32_t>& materialToTexture);     //TODO: Use reference?

    static void      createTextureImage(const std::string &filename, vk::Image& ref, VmaAllocation& allocRef);
    static uint32_t  createTextureDescriptor(vk::ImageView textureImage);
    static stbi_uc*  loadTextuerFile(const std::string &filename, int* width, int* height, vk::DeviceSize* imageSize);
public:
    static void setVulkanStructures(VmaAllocator*vma,vk::PhysicalDevice*physiscalDev,Device*dev,vk::Queue*transQueue,vk::CommandPool*transCmdPool);
    static ImageData createTexture(const std::string &filename);
    static void cleanupTexture(ImageData& ref);
};