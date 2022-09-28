#pragma once 

#include <cstdint>
#include <unordered_map>
#include <string>
#include "ResourceManagerStructs.hpp"
#include "../graphics/Mesh.hpp"
#include "../graphics/Texture.hpp"
#include "../dev/Log.hpp"
#include "loaders/TextureLoader.hpp"
#include "loaders/MeshLoader.hpp"

class Engine;
struct ImageData;   //Defined in MeshLoader

class ResourceManager{
private:
    /// VulkanRenderer takes care of cleanups
    friend class VulkanRenderer; 
    std::unordered_map<std::string, uint32_t> meshPaths;
    std::unordered_map<std::string, uint32_t> texturePaths;
    
    std::unordered_map<uint32_t, Mesh>  meshes;
    std::unordered_map<uint32_t, Texture> textures;

    MeshLoader      meshLoader;
    TextureLoader   textureLoader;

    void cleanup();
public:
    ResourceManager() = default;
    void init(
        VmaAllocator * vma,
        vk::PhysicalDevice* physiscalDev,
        Device* dev, vk::Queue* transQueue,
        vk::CommandPool* transCmdPool,
        VulkanRenderer* vulkanRenderer);

    uint32_t addMesh(std::string&& meshPath);
    uint32_t addTexture(std::string&& texturePath);

    Mesh&         getMesh(uint32_t id);
    Texture&    getTexture(uint32_t id);
};

inline Mesh& ResourceManager::getMesh(uint32_t id)
{
    auto map_iterator = this->meshes.find(id);
    if(this->meshes.end() == map_iterator)
    {
        Log::error("getMesh failed to find a mesh with the given ID : " 
            + std::to_string(id));        
    }
    return map_iterator->second;
}

inline Texture& ResourceManager::getTexture(uint32_t id)
{
    auto map_iterator = this->textures.find(id);
    if(this->textures.end() == map_iterator)
    {
        Log::error("getMesh failed to find a mesh with the given ID : " 
            + std::to_string(id));        
    }
    return map_iterator->second;
}
