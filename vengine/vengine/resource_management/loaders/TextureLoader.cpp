#include "pch.h"
#include "TextureLoader.hpp"
#include "MeshLoader.hpp"
#include "../Configurator.hpp"
#include "stb_image.h"
#include "tracy/Tracy.hpp"
#include "../../graphics/Texture.hpp"
#include "../../graphics/Buffer.hpp"
#include "../../dev/StringHelper.hpp"
#include "assimp/scene.h"
#include "assimp/material.h"
#include "assimp/Importer.hpp" 
#include "../ResourceManager.hpp" // Importing mesh with Assimp needs to add Texture resources
#include <span>

void TextureLoader::init(
    VmaAllocator *vma,
    PhysicalDevice *physiscalDev,
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

void TextureLoader::assimpGetTextures(const aiScene *scene, std::vector<std::string> &diffuseTextures, std::vector<std::string> &emissiveTextures) 
{
    diffuseTextures.resize(scene->mNumMaterials);
    emissiveTextures.resize(scene->mNumMaterials);
    int textureIndex =
        0; // index of the texture that corresponds to the Diffuse material

    // For each material, if texture file name exists, store it in vector
    for (auto *material :
        std::span<aiMaterial *>(scene->mMaterials, scene->mNumMaterials)) 
    {
        // Get the path of the texture file
        aiString path;
        // Check for a diffuse texture
        if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0) 
        {
            if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS) 
            {
                diffuseTextures[textureIndex] = std::string(path.C_Str());
            }
        }
        if (material->GetTextureCount(aiTextureType_EMISSIVE) != 0)
        {
            if (material->GetTexture(aiTextureType_EMISSIVE, 0, &path) == AI_SUCCESS)
            {
                emissiveTextures[textureIndex] = std::string(path.C_Str());
            }
        }

        textureIndex++;
    }
}

void TextureLoader::assimpTextureImport(
    const aiScene* scene, 
    const std::string& texturesFolderPath,
    std::vector<uint32_t>& materialToTexture)
{
    // Get vector of all materials
    std::vector<std::string> diffuseNames;
    std::vector<std::string> emissiveNames;
    this->assimpGetTextures(scene, diffuseNames, emissiveNames);

    // Handle empty texture
    materialToTexture.resize(diffuseNames.size());
    for (size_t i = 0; i < diffuseNames.size(); i++) {

        if (diffuseNames[i].empty()) {
            // Use default textures for models if textures are missing
            materialToTexture[i] = 0;
        }
        else 
        {
            // Extract the name from texture path, and add custom folder path
            this->processTextureName(diffuseNames[i], texturesFolderPath);
            if (emissiveNames[i].empty())
                emissiveNames[i] = "vengine_assets/textures/White.png";
            else
                this->processTextureName(emissiveNames[i], texturesFolderPath);

            

            // Add material
            uint32_t addedMaterialIndex =
                this->resourceMan->addMaterial(
                    this->resourceMan->addTexture(diffuseNames[i].c_str()),
                    this->resourceMan->addTexture("vengine_assets/textures/NoSpecular.png"),
                this->resourceMan->addTexture(emissiveNames[i].c_str())
                );

            // Create texture, use the index returned by our createTexture function
            materialToTexture[i] = addedMaterialIndex;
        }
    }
}

Texture TextureLoader::createTexture(
    const std::string &filename, 
    const TextureSettings& textureSettings,
    const uint32_t& textureSamplerIndex)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //: NOLINT
#endif
    Texture createdTexture;

    // Load the image file
    int width = 0;
    int height = 0;
    stbi_uc* imageData =
        this->loadTextureFile(filename, &width, &height);

    // Create vulkan texture
    createdTexture.create(
        *this->importStructs.physicalDevice,
        *this->importStructs.device,
        *this->importStructs.vma,
        *this->importStructs.transferQueue,
        *this->importStructs.transferCommandPool,
        imageData,
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height),
        textureSettings,
        textureSamplerIndex
    );

    // Free image data allocated through stb_image.h
    stbi_image_free(imageData);

    return createdTexture;
}

stbi_uc *TextureLoader::loadTextureFile(const std::string &filename, int *width,
                                        int *height) 
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

    return image;
}

void TextureLoader::processTextureName(
    std::string& filePathFromModel, 
    const std::string& texturesFolderPath)
{
    // Make sure the filename string is actually 
    // supposed to be processed
    if (texturesFolderPath == "")
    {
        return;
    }

    // Replace '\' with '/'
    std::string tempFilePathFromModel = filePathFromModel;
    for (size_t i = 0; i < tempFilePathFromModel.length(); ++i)
    {
        if (tempFilePathFromModel[i] == '\\')
        {
            tempFilePathFromModel[i] = '/';
        }
    }

    // Split filename to make folderStrings contain folder/texture names 
    // in std::vector elements
    std::vector<std::string> folderStrings;
    StringHelper::splitString(tempFilePathFromModel, '/', folderStrings);

    // Add custom folder path in front of texture name
    filePathFromModel = texturesFolderPath + "/" + folderStrings[folderStrings.size() - 1];
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