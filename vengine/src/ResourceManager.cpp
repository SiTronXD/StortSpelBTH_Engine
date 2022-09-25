#include "ResourceManager.hpp"
#include "Log.hpp"
#include "Configurator.hpp"
#include "MeshLoader.hpp"
#include "TextureLoader.hpp"

uint32_t ResourceManager::addMesh(std::string&& meshPath)
{        
    using namespace vengine_helper::config;    
    
    uint32_t prevSize = this->meshPaths.size();
    this->meshPaths.insert({meshPath,prevSize}); //NOTE: prevSize as key only works if we never remove resources the map...

    // If exists, return key of existing mesh
    if(this->meshPaths.size() == prevSize)
    {
        Log::warning("Mesh \""+meshPath+"\" was already added!"); //TODO: should be able to log what mesh
        
        //TODO: Check if Value of meshPath is different from the value which is returned when using same key... i.e. check for collision
        //if()

        return this->meshes.find(
                this->meshPaths.find(meshPath)->second
        )->first;
    }    

    auto model = MeshLoader::createMesh(DEF<std::string>(P_MODELS) + meshPath); //NOTE: meshes.size as key only works if we never remove resources the map...

    // Create mesh, insert into map of meshes
    meshes.insert({
        meshes.size(),
        std::move(model)}        
        ); 

    return meshes.size() -1;
}

uint32_t ResourceManager::addTexture(std::string&& texturePath)
{ 
    using namespace vengine_helper::config;    
    
    uint32_t prevSize = this->texturePaths.size();

    //NOTE: prevSize as key only works if we never remove resources the map...
    this->texturePaths.insert({texturePath,prevSize}); 

    // If exists, return key of existing mesh
    if(this->texturePaths.size() == prevSize)
    {
        Log::warning("Texture ["+texturePath+"] was already added!");
        
        return this->textures.find(
            this->texturePaths.find(texturePath)->second
            )->first;
    }

    //NOTE: meshes.size as key only works if we never remove resources the map...
    auto textureResource = TextureLoader::createTexture(DEF<std::string>(P_TEXTURES) + texturePath); 

    // Create mesh, insert into map of meshes
    textures.insert({
        textures.size(),
        textureResource});       

    return textures.size() -1;
}

void ResourceManager::cleanup()
{
    for(auto & keyVal : this->meshes)
    {                
        keyVal.second.cleanup();
    }
    for(auto & i : this->textures)
    {                   
        TextureLoader::cleanupTexture(i.second);
    }
}