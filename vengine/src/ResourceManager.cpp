#include "ResourceManager.hpp"
#include "Model.hpp"
#include "Log.hpp"
#include "MeshLoader.hpp"

std::unordered_map<std::string, uint32_t>   ResourceManager::meshPaths;
std::unordered_map<std::string, uint32_t>   ResourceManager::texturePaths;
std::unordered_map<uint32_t, NewModel>      ResourceManager::meshes;
std::unordered_map<uint32_t, ImageData>     ResourceManager::textures;

uint32_t ResourceManager::addTexture(std::string&& texturePath)
{ 
    using namespace vengine_helper::config;    
    
    uint32_t prevSize = ResourceManager::texturePaths.size();
    ResourceManager::texturePaths.insert({texturePath,prevSize}); //NOTE: prevSize as key only works if we never remove resources the map...

    // If exists, return key of existing mesh
    if(ResourceManager::texturePaths.size() == prevSize)
    {
        Log::warning("Texture [TODO: insert name of mesh file] was already added!"); //TODO: should be able to log what mesh
        
        
        return ResourceManager::textures.find(
                ResourceManager::texturePaths.find(texturePath)->second
        )->first;
    }

    auto textureResource = TextureLoader::createTexture(DEF<std::string>(P_TEXTURES) + texturePath); //NOTE: meshes.size as key only works if we never remove resources the map...

    // Create mesh, insert into map of meshes
    textures.insert({
        textures.size(),
        textureResource});       

    return textures.size() -1;
}




void ResourceManager::cleanup()
{
    for(auto & i : ResourceManager::textures)
    {                   
        TextureLoader::cleanupTexture(i.second);
    }
}
