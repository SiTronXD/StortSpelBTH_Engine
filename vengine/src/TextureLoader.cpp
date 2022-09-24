#include "TextureLoader.hpp"
#include "Configurator.hpp"
#include "stb_image.h"
#include "tracy/Tracy.hpp"
#include "Texture.hpp"
#include "assimp/scene.h"
#include "assimp/material.h"
#include "ResourceManager.hpp" // Importing mesh with Assimp needs to add Texture resources
#include <span>

#include "VulkanRenderer.hpp"

VulkanImportStructs TextureLoader::importStructs;
VulkanRenderer *TextureLoader::TEMP = nullptr; 

void TextureLoader::init(VmaAllocator *vma,
                                        vk::PhysicalDevice *physiscalDev,
                                        Device *dev, vk::Queue *transQueue,
                                        vk::CommandPool *transCmdPool) 
{
    TextureLoader::importStructs.vma = vma;
    TextureLoader::importStructs.physicalDevice = physiscalDev;
    TextureLoader::importStructs.device = dev;
    TextureLoader::importStructs.transferQueue = transQueue;
    TextureLoader::importStructs.transferCommandPool = transCmdPool;
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
    const aiScene *scene, std::vector<uint32_t> &materialToTexture) 
{
    // Get vector of all materials
    std::vector<std::string> textureNames =
        TextureLoader::assimpGetTextures(scene);

    // Handle empty texture
    materialToTexture.resize(textureNames.size());
    for (size_t i = 0; i < textureNames.size(); i++) {

    if (textureNames[i].empty()) {
        // Use default textures for models if textures are missing
        materialToTexture[i] = 0;
    } else {
        // Create texture, use the index returned by our createTexture function
        materialToTexture[i] =
            ResourceManager::addTexture(textureNames[i].c_str());
    }
    }
}

ImageData TextureLoader::createTexture(const std::string &filename) {
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //: NOLINT
#endif
    ImageData imageData;

    // Create Texture Image and get its Location in array
    TextureLoader::createTextureImage(filename, imageData.image,
                                    imageData.imageMemory);

    // Create Image View
    imageData.imageView = Texture::createImageView(
        *TextureLoader::importStructs.device,
        // textureImages[textureImageLoc],          // The location of the Image
        // in our textureImages vector
        imageData.image, // The location of the Image in our textureImages vector
        vk::Format::eR8G8B8A8Unorm, // Format for rgba
        vk::ImageAspectFlagBits::eColor);

    // Create Texture Descriptor
    imageData.descriptorLocation =
        TextureLoader::createTextureDescriptor(imageData.imageView);

    // Return index of Texture Descriptor that was just created
    return imageData;
}

void TextureLoader::cleanupTexture(ImageData &ref) 
{
    TextureLoader::importStructs.device->getVkDevice().destroyImageView(
        ref.imageView);
    TextureLoader::importStructs.device->getVkDevice().destroyImage(ref.image);
    vmaFreeMemory(*TextureLoader::importStructs.vma, ref.imageMemory);
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

void TextureLoader::createTextureImage(const std::string &filename,
                                       vk::Image &imageRef,
                                       VmaAllocation &allocRef) {
#ifndef VENGINE_NO_PROFILING
  ZoneScoped; //: NOLINT
#endif
    // Load the image file
    int width = 0;
    int height = 0;
    vk::DeviceSize imageSize = 0;
    stbi_uc *imageData =
        TextureLoader::loadTextureFile(filename, &width, &height, &imageSize);

    // Create Staging buffer to hold loaded data, ready to copy to device
    vk::Buffer imageStagingBuffer = nullptr;
    VmaAllocation imageStagingBufferMemory = nullptr;
    VmaAllocationInfo allocInfo;

    vengine_helper::createBuffer(createBufferData{
        .physicalDevice = *TextureLoader::importStructs.physicalDevice,
        .device = TextureLoader::importStructs.device->getVkDevice(),
        .bufferSize = imageSize,
        .bufferUsageFlags = vk::BufferUsageFlagBits::eTransferSrc,
        .bufferProperties =
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
            VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .buffer = &imageStagingBuffer,
        .bufferMemory = &imageStagingBufferMemory,
        .allocationInfo = &allocInfo,
        .vma = TextureLoader::importStructs.vma});

    void *data = nullptr;

    if (vmaMapMemory(*TextureLoader::importStructs.vma, imageStagingBufferMemory,
                    &data) != VK_SUCCESS) {
    throw std::runtime_error(
        "Failed to allocate Mesh Staging Texture Image Buffer Using VMA!");
    };

    // memcpy(allocInfo.pMappedData, imageData, imageSize);
    memcpy(data, imageData, imageSize);
    vmaUnmapMemory(*TextureLoader::importStructs.vma, imageStagingBufferMemory);

    // Free image data allocated through stb_image.h
    stbi_image_free(imageData);

    imageRef = Texture::createImage(
        *TextureLoader::importStructs.vma,
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
        .property =
            vk::MemoryPropertyFlagBits::eDeviceLocal, // Image should only be
                                                        // accesable on the GPU
        .imageMemory = &allocRef

        },
        filename // Describing what image is being created, for debug purposes...
    );

    // - COPY THE DATA TO THE IMAGE -
    // Transition image to be in the DST, needed by the Copy Operation (Copy
    // assumes/needs image Layout to be in vk::ImageLayout::eTransferDstOptimal
    // state)
    vengine_helper::transitionImageLayout(
        TextureLoader::importStructs.device->getVkDevice(),
        *TextureLoader::importStructs.transferQueue, // Same as graphics Queue
        *TextureLoader::importStructs.transferCommandPool,
        imageRef,                    // Image to transition the layout on
        vk::ImageLayout::eUndefined, // Image Layout to transition the image from
        vk::ImageLayout::eTransferDstOptimal); // Image Layout to transition the
                                                // image to

    // Copy Data to image
    vengine_helper::copyImageBuffer(
        TextureLoader::importStructs.device->getVkDevice(),
        *TextureLoader::importStructs.transferQueue,
        *TextureLoader::importStructs.transferCommandPool, imageStagingBuffer,
        imageRef, width, height);

    // Transition iamge to be shader readable for shader usage
    vengine_helper::transitionImageLayout(
        TextureLoader::importStructs.device->getVkDevice(),
        *TextureLoader::importStructs.transferQueue,
        *TextureLoader::importStructs.transferCommandPool, imageRef,
        vk::ImageLayout::eTransferDstOptimal, // Image layout to transition the
                                            // image from; this is the same as
                                            // we transition the image too
                                            // before we copied buffer!
        vk::ImageLayout::eShaderReadOnlyOptimal); // Image Layout to transition
                                                // the image to; in order for
                                                // the Fragment Shader to read
                                                // it!

    // Destroy and Free the staging buffer + staging buffer memroy
    TextureLoader::importStructs.device->getVkDevice().destroyBuffer(
        imageStagingBuffer);

    vmaFreeMemory(*TextureLoader::importStructs.vma, imageStagingBufferMemory);
}

uint32_t TextureLoader::createTextureDescriptor(vk::ImageView textureImage) 
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //: NOLINT
#endif
    vk::DescriptorSet descriptorSet = nullptr;

    // Descriptor Set Allocation Info
    vk::DescriptorSetAllocateInfo setAllocateInfo;
    setAllocateInfo.setDescriptorPool(
        TextureLoader::TEMP
            ->samplerDescriptorPool); // TODO: Do not use Pointer to
                                    // VulkanRenderer instance...
    setAllocateInfo.setDescriptorSetCount(uint32_t(1));
    setAllocateInfo.setPSetLayouts(
        &TextureLoader::TEMP->samplerDescriptorSetLayout);

    // Allocate Descriptor Sets
    descriptorSet =
        TextureLoader::importStructs.device->getVkDevice().allocateDescriptorSets(
            setAllocateInfo)[0];

    // Tedxture Image info
    vk::DescriptorImageInfo imageInfo;
    imageInfo.setImageLayout(
        vk::ImageLayout::eShaderReadOnlyOptimal); // The Image Layout when it is
                                                // in use
    imageInfo.setImageView(textureImage);         // Image to be bind to set
    imageInfo.setSampler(
        TextureLoader::TEMP
            ->textureSampler); // the Sampler to use for this Descriptor Set

    // Descriptor Write Info
    vk::WriteDescriptorSet descriptorWrite;
    descriptorWrite.setDstSet(descriptorSet);
    descriptorWrite.setDstArrayElement(uint32_t(0));
    descriptorWrite.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
    descriptorWrite.setDescriptorCount(uint32_t(1));
    descriptorWrite.setPImageInfo(&imageInfo);

    // Update the new Descriptor Set
    TextureLoader::importStructs.device->getVkDevice().updateDescriptorSets(
        uint32_t(1), &descriptorWrite, uint32_t(0), nullptr);

    // Add descriptor Set to our list of descriptor Sets
    TextureLoader::TEMP->samplerDescriptorSets.push_back(descriptorSet);

    // Return the last created Descriptor set
    return static_cast<uint32_t>(
        TextureLoader::TEMP->samplerDescriptorSets.size() - 1);
}