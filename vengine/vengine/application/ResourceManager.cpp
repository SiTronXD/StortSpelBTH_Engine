#include "ResourceManager.hpp"
#include "../loaders/Configurator.hpp"
#include "../loaders/MeshLoader.hpp"
#include "../loaders/TextureLoader.hpp"

uint32_t ResourceManager::addMesh(std::string&& meshPath)
{        
    using namespace vengine_helper::config;    
    
    uint32_t prevSize = this->meshPaths.size();
    //NOTE: prevSize as key only works if we never remove resources the map...
    this->meshPaths.insert({meshPath,prevSize}); 

    // If exists, return key of existing mesh
    if(this->meshPaths.size() == prevSize)
    {
        //TODO: should be able to log what mesh
        Log::warning("Mesh \""+meshPath+"\" was already added!"); 
        
        // Check if Collision happened.
        // - Check if the meshPaths-map bucket for specific ID has more than 1 element
        uint32_t bucketSize = this->meshPaths.bucket_size(
            // Get bucket index based on the given key
            this->meshPaths.bucket(meshPath));

        if(bucketSize != 1) 
        {assert(false && "ResourceManager::Collision in map[meshPaths]");}        
        

        return this->meshes.find(
                this->meshPaths.find(meshPath)->second
        )->first;
    }    

    //NOTE: meshes.size as key only works if we never remove resources the map...
    auto model = MeshLoader::createMesh(DEF<std::string>(P_MODELS) + meshPath); //TODO: Move this into the map insert function call...

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
        
        // Check if Collision happened.
        // - Check if the texturePaths-map bucket for specific ID has more than 1 element
        uint32_t bucketSize = this->texturePaths.bucket_size(
            // Get bucket index based on the given key
            this->texturePaths.bucket(texturePath));

        if(bucketSize != 1) 
        {assert(false && "ResourceManager::Collision in map[texturePaths]");}        
        
        return this->textures.find(
            this->texturePaths.find(texturePath)->second
            )->first;
    }

    //NOTE: meshes.size as key only works if we never remove resources the map...
    auto textureResource = 
        TextureLoader::createTexture(DEF<std::string>(P_TEXTURES) + texturePath); 

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