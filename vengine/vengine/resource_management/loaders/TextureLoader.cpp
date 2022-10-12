#include "TextureLoader.hpp"
#include "../Configurator.hpp"
#include "stb_image.h"
#include "tracy/Tracy.hpp"
#include "../../graphics/Texture.hpp"
#include "assimp/scene.h"
#include "assimp/material.h"
#include "assimp/Importer.hpp" 
#include "../ResourceManager.hpp" // Importing mesh with Assimp needs to add Texture resources
#include <span>

#include "../../graphics/Buffer.hpp"
#include "../../graphics/VulkanRenderer.hpp"
#include "MeshLoader.hpp"

void TextureLoader::init(VmaAllocator *vma,
                                        vk::PhysicalDevice *physiscalDev,
                                        Device *dev, vk::Queue *transQueue,
                                        vk::CommandPool *transCmdPool,
                                        ResourceManager* resourceMan) 
{
    this->importStructs.vma = vma;
    this->importStructs.physicalDevice = physiscalDev;
    this->importStructs.device = dev;
    this->importStructs.transferQueue = transQueue;
    this->importStructs.transferCommandPool = transCmdPool;
    this->resourceMan = resourceMan;
}

std::vector<std::string> TextureLoader::assimpGetTextures(const aiScene *scene) 
{
    std::vector<std::string> textureList(scene->mNumMaterials);
    int textureIndex =
        0; /// index of the texture that corresponds to the Diffuse material

    /// For each material, if texture file name exists, store it in vector
    for (auto *material :
        std::span<aiMaterial *>(scene->mMaterials, scene->mNumMaterials)) {

    /// Check for a Diffure Texture
    if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
        /// get the Path of the texture file
        aiString path;
        if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS) {
            textureList[textureIndex] = std::string(path.C_Str());
        }
    }
    textureIndex++; /// Which Material the
    }
    return textureList;
}

void TextureLoader::assimpTextureImport(
    const aiScene* scene, std::vector<uint32_t>& materialToTexture)
{
    // Get vector of all materials
    std::vector<std::string> textureNames =
        this->assimpGetTextures(scene);

    // Handle empty texture
    materialToTexture.resize(textureNames.size());
    for (size_t i = 0; i < textureNames.size(); i++) {

        if (textureNames[i].empty()) {
            // Use default textures for models if textures are missing
            materialToTexture[i] = 0;
        }
        else {
            // Create texture, use the index returned by our createTexture function
            materialToTexture[i] =
                this->resourceMan->addTexture(textureNames[i].c_str());
        }
    }
}

Texture TextureLoader::createTexture(
    const std::string &filename, 
    const uint32_t& textureSamplerIndex)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //: NOLINT
#endif
    Texture createdTexture(
        *this->importStructs.device, 
        *this->importStructs.vma
    );

    // Create Texture Image
    uint32_t width = 0;
    uint32_t height = 0;
    this->createTextureImage(
        filename, 
        createdTexture.image, 
        createdTexture.imageMemory,
        width,
        height
    );

    // Create Image View
    createdTexture.init(
        Texture::createImageView(
            *this->importStructs.device,
            createdTexture.image,
            vk::Format::eR8G8B8A8Unorm, // Format for rgba
            vk::ImageAspectFlagBits::eColor
        ),
        width, 
        height,
        textureSamplerIndex
    );

    // Return index of Texture Descriptor that was just created
    return createdTexture;
}

stbi_uc *TextureLoader::loadTextureFile(const std::string &filename, int *width,
                                        int *height,
                                        vk::DeviceSize *imageSize) 
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //: NOLINT
#endif
    // Number of Channels the image uses, will not be used but could be used in
    // the future
    int channels = 0;
    using namespace vengine_helper::config;
    // Load pixel Data from file to image
    stbi_uc *image = stbi_load(
        filename.c_str(), width, height,
        &channels, // In case we want to  use channels, its stored in channels
        STBI_rgb_alpha); // force image to be in format : RGBA

    if (image == nullptr) {
    throw std::runtime_error("Failed to load a Texture file! (" + filename +
                                ")");
    }

    // Calculate image sisze using given and known data
    *imageSize = static_cast<uint32_t>((*width) * (*height) *
                                        4); // width times height gives us size per
                                            // channel, we have 4 channels! (rgba)

    return image;
}

void TextureLoader::createTextureImage(
    const std::string &filename,
    vk::Image &imageRef,
    VmaAllocation &allocRef,
    uint32_t& outputWidth,
    uint32_t& outputHeight) 
{
#ifndef VENGINE_NO_PROFILING
  ZoneScoped; //: NOLINT
#endif
    // Load the image file
    int width = 0;
    int height = 0;
    vk::DeviceSize imageSize = 0;
    stbi_uc *imageData =
        this->loadTextureFile(filename, &width, &height, &imageSize);

    outputWidth = static_cast<uint32_t>(width);
    outputHeight = static_cast<uint32_t>(height);

    // Create Staging buffer to hold loaded data, ready to copy to device
    vk::Buffer imageStagingBuffer = nullptr;
    VmaAllocation imageStagingBufferMemory = nullptr;
    VmaAllocationInfo allocInfo;

    Buffer::createBuffer(BufferCreateData{
        .bufferSize = imageSize,
        .bufferUsageFlags = vk::BufferUsageFlagBits::eTransferSrc,
        .bufferProperties =
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
            VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .buffer = &imageStagingBuffer,
        .bufferMemory = &imageStagingBufferMemory,
        .allocationInfo = &allocInfo,
        .vma = this->importStructs.vma});

    void *data = nullptr;

    if (vmaMapMemory(*this->importStructs.vma, imageStagingBufferMemory,
                    &data) != VK_SUCCESS) {
    throw std::runtime_error(
        "Failed to allocate Mesh Staging Texture Image Buffer Using VMA!");
    };

    // memcpy(allocInfo.pMappedData, imageData, imageSize);
    memcpy(data, imageData, imageSize);
    vmaUnmapMemory(*this->importStructs.vma, imageStagingBufferMemory);

    // Free image data allocated through stb_image.h
    stbi_image_free(imageData);

    imageRef = Texture::createImage(
        *this->importStructs.vma,
        {.width = static_cast<uint32_t>(width),
        .height = static_cast<uint32_t>(height),
        .format = vk::Format::eR8G8B8A8Unorm, // use Alpha channel even if image
                                                // does not have...
        .tiling = vk::ImageTiling::eOptimal,  // Same value as the Depth Buffer
                                                // uses (Dont know if it has to be)
        .useFlags =
            vk::ImageUsageFlagBits::eTransferDst // Data should be transfered to
                                                // the GPU, from the staging
                                                // buffer
            | vk::ImageUsageFlagBits::eSampled,  // This image will be Sampled by
                                                // a Sampler!
        .imageMemory = &allocRef

        },
        filename // Describing what image is being created, for debug purposes...
    );

    // - COPY THE DATA TO THE IMAGE -
    // Transition image to be in the DST, needed by the Copy Operation (Copy
    // assumes/needs image Layout to be in vk::ImageLayout::eTransferDstOptimal
    // state)
    Texture::transitionImageLayout(
        *this->importStructs.device,
        *this->importStructs.transferQueue, // Same as graphics Queue
        *this->importStructs.transferCommandPool,
        imageRef,                    // Image to transition the layout on
        vk::ImageLayout::eUndefined, // Image Layout to transition the image from
        vk::ImageLayout::eTransferDstOptimal); // Image Layout to transition the
                                                // image to

    // Copy Data to image
    Buffer::copyBufferToImage(
        this->importStructs.device->getVkDevice(),
        *this->importStructs.transferQueue,
        *this->importStructs.transferCommandPool, imageStagingBuffer,
        imageRef, width, height);

    // Transition iamge to be shader readable for shader usage
    Texture::transitionImageLayout(
        *this->importStructs.device,
        *this->importStructs.transferQueue,
        *this->importStructs.transferCommandPool, imageRef,
        vk::ImageLayout::eTransferDstOptimal, // Image layout to transition the
                                            // image from; this is the same as
                                            // we transition the image too
                                            // before we copied buffer!
        vk::ImageLayout::eShaderReadOnlyOptimal); // Image Layout to transition
                                                // the image to; in order for
                                                // the Fragment Shader to read
                                                // it!

    // Destroy and Free the staging buffer + staging buffer memroy
    this->importStructs.device->getVkDevice().destroyBuffer(
        imageStagingBuffer);

    vmaFreeMemory(*this->importStructs.vma, imageStagingBufferMemory);
}

bool TextureLoader::doesTextureExist(const std::string& filePath)
{
    // Easy and cross-platform way of checking if the texture exists

    int width = 0;
    int height = 0;
    int channels = 0;

    // Load pixel data from file to image
    stbi_uc* imageData = stbi_load(
        filePath.c_str(), 
        &width, 
        &height,
        &channels,
        STBI_rgb_alpha
    ); 

    bool doesImageExist = imageData != nullptr;

    // Deallocate pixel data
    stbi_image_free(imageData);

    return doesImageExist;
}