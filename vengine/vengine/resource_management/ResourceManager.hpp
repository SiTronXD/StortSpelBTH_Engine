#pragma once 

#include <cstdint>
#include <unordered_map>
#include <string>
#include "ResourceManagerStructs.hpp"
#include "../graphics/Mesh.hpp"
#include "../graphics/Texture.hpp"
#include "../graphics/TextureSampler.hpp"
#include "../dev/Log.hpp"
#include "loaders/TextureLoader.hpp"
#include "loaders/MeshLoader.hpp"
#include "loaders/ColliderLoader.hpp"

class Engine;
struct ImageData;   //Defined in MeshLoader

class ResourceManager
{
private:
    /// VulkanRenderer takes care of cleanups
    friend class VulkanRenderer; 
    std::unordered_map<std::string, uint32_t> meshPaths;
    std::unordered_map<std::string, uint32_t> texturePaths;
    std::unordered_map<std::string, uint32_t> samplerSettings;
	std::unordered_map<std::string, uint32_t> collisionPaths;
    std::unordered_map<std::string, uint32_t> soundPaths;

    std::unordered_map<uint32_t, Mesh>  meshes;
	std::unordered_map<uint32_t, std::vector<ColliderDataRes>> collisionsData;
    std::unordered_map<uint32_t, Texture> textures;
    std::unordered_map<uint32_t, TextureSampler> textureSamplers;
    std::vector<uint32_t> audioBuffers; // Is this even needed ?

    MeshLoader      meshLoader;
	ColliderLoader  collisionLoader;
    TextureLoader   textureLoader;

    Device* dev = nullptr;

    void cleanup();
public:
    ResourceManager() = default;
    void init(
        VmaAllocator * vma,
        vk::PhysicalDevice* physiscalDev,
        Device* dev, vk::Queue* transQueue,
        vk::CommandPool* transCmdPool,
        VulkanRenderer* vulkanRenderer);
		
    void cleanUp();

    uint32_t addMesh(std::string&& meshPath, 
        std::string&& texturesPath = "");
    uint32_t addTexture(std::string&& texturePath,
        const TextureSettings& textureSettings = {});
	uint32_t addCollisionShapeFromMesh(std::string&& collisionPath);
	uint32_t addAnimations(const std::vector<std::string>& paths, std::string&& texturesPath = "");
    bool mapAnimations(uint32_t meshid, const std::vector<std::string>& names);
    uint32_t addSound(std::string&& soundPath);

    Mesh& getMesh(uint32_t id);
    Texture& getTexture(uint32_t id);
    TextureSampler& getTextureSampler(uint32_t id);
	std::vector<ColliderDataRes> getCollisionShapeFromMesh(std::string&& collisionPath);
	std::vector<ColliderDataRes> getCollisionShapeFromMesh(uint32_t id);

    size_t getNumMeshes();
    size_t getNumTextures();
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

inline TextureSampler& ResourceManager::getTextureSampler(uint32_t id)
{
    auto map_iterator = this->textureSamplers.find(id);
    if (this->textureSamplers.end() == map_iterator)
    {
        Log::error("getTextureSampler failed to find a sampler with the given ID : "
            + std::to_string(id));
    }
    return map_iterator->second;
}

inline std::vector<ColliderDataRes> ResourceManager::getCollisionShapeFromMesh(std::string&& collisionPath)
{
	if (this->collisionPaths.count(collisionPath) != 0)
	{
		return getCollisionShapeFromMesh(this->collisionPaths[collisionPath]);
    }
	else
	{
		Log::error("Failed to find Collision with given name : " + collisionPath);
		return std::vector<ColliderDataRes>();
    }
}

inline std::vector<ColliderDataRes> ResourceManager::getCollisionShapeFromMesh(uint32_t id)
{
	auto map_iterator = this->collisionsData.find(id);
	if (this->collisionsData.end() == map_iterator)
	{
		Log::error("Failed to find Collision with the given ID : " + std::to_string(id));
	}
	return map_iterator->second;
}

inline size_t ResourceManager::getNumMeshes() 
{
    return this->meshes.size();
}

inline size_t ResourceManager::getNumTextures() 
{
    return this->textures.size();
}