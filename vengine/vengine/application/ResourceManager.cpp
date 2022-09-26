#include "ResourceManager.hpp"
#include "../loaders/Configurator.hpp"
#include "../loaders/MeshLoader.hpp"
#include "../loaders/TextureLoader.hpp"

uint32_t ResourceManager::addMesh(std::string&& meshPath)
{        
    using namespace vengine_helper::config;

    if(this->meshPaths.count(meshPath) != 0)
    {
        //TODO: should be able to log what mesh
        Log::warning("Mesh \""+meshPath+"\" was already added!"); 
        
        // Check if Collision happened.
        // - Check if the meshPaths-map bucket for specific ID has more than 1 element
        uint32_t bucketSize = this->meshPaths.bucket_size(
            // Get bucket index based on the given key
            this->meshPaths.bucket(meshPath));

        if(bucketSize != 1) 
        {Log::error("ResourceManager::Collision in meshPaths["+meshPath+"]");}        
        
        return this->meshPaths[meshPath];        
    } 
        
    //NOTE: prevSize as key only works if we never remove resources the map...
    this->meshPaths.insert({meshPath,this->meshPaths.size()}); 

    //NOTE: meshes.size as key only works if we never remove resources the map...    
    // Create mesh, insert into map of meshes
    meshes.insert({
        meshes.size(),
        MeshLoader::createMesh(DEF<std::string>(P_MODELS) + meshPath)}        
        ); 

    return meshes.size() -1;
}

uint32_t ResourceManager::addTexture(std::string&& texturePath)
{ 
    using namespace vengine_helper::config;

    if(this->texturePaths.count(texturePath) != 0)
    {
        Log::warning("Texture ["+texturePath+"] was already added!");
        
        // Check if Collision happened.
        // - Check if the texturePaths-map bucket for specific ID has more than 1 element
        uint32_t bucketSize = this->texturePaths.bucket_size(
            // Get bucket index based on the given key
            this->texturePaths.bucket(texturePath));

        if(bucketSize != 1) 
        {Log::error("ResourceManager::Collision in texturePaths["+texturePath+"]");}                
        
        return this->texturePaths[texturePath];
    }
    
    //NOTE: texturePaths.size() as key only works if we never remove resources the map...
    this->texturePaths.insert({texturePath,this->texturePaths.size()}); 

    //NOTE: meshes.size as key only works if we never remove resources the map...    
    // Create mesh, insert into map of meshes
    textures.insert({
        textures.size(),
        TextureLoader::createTexture(DEF<std::string>(P_TEXTURES) + texturePath)});

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