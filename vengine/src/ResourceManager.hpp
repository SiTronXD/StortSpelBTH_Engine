#pragma once 

#include <cstdint>
#include <unordered_map>
#include <string>
#include "NewModel.hpp"
#include "ResourceManagerStructs.hpp"
#include "Log.hpp"

class Engine;
struct ImageData;   //Defined in MeshLoader

class ResourceManager{
private:
    /// VulkanRenderer takes care of cleanups
    friend class VulkanRenderer; 
    std::unordered_map<std::string, uint32_t> meshPaths;
    std::unordered_map<std::string, uint32_t> texturePaths;
    
    std::unordered_map<uint32_t, NewModel>  meshes;
    std::unordered_map<uint32_t, ImageData> textures;

    void cleanup();
public:    
    uint32_t addMesh(std::string&& meshPath);
    uint32_t addTexture(std::string&& texturePath);

    NewModel&     getMesh(uint32_t id);
    ImageData&    getTexture(uint32_t id);
};

inline NewModel& ResourceManager::getMesh(uint32_t id)
{
    auto map_iterator = this->meshes.find(id);
    if(this->meshes.end() == map_iterator)
    {
        Log::error("getMesh failed to find a mesh with the given ID : " 
            + std::to_string(id));
        assert(false);
    }
    return map_iterator->second;
}

inline ImageData& ResourceManager::getTexture(uint32_t id)
{
    auto map_iterator = this->textures.find(id);
    if(this->textures.end() == map_iterator)
    {
        Log::error("getMesh failed to find a mesh with the given ID : " 
            + std::to_string(id));
        assert(false);
    }
    return map_iterator->second;
}
