#include "ResourceManager.hpp"
#include "Model.hpp"
#include "Log.hpp"
#include "MeshLoader.hpp"

std::unordered_map<std::string, uint32_t>   ResourceManager::meshPaths;
std::unordered_map<std::string, uint32_t>   ResourceManager::texturePaths;




void ResourceManager::cleanup()
{
    for(auto & i : ResourceManager::textures)
    {                   
        TextureLoader::cleanupTexture(i.second);
    }
}
