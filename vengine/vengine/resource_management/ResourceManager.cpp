#include "ResourceManager.hpp"
#include "Configurator.hpp"
#include "loaders/MeshLoader.hpp"
#include "loaders/TextureLoader.hpp"

void ResourceManager::init(
    VmaAllocator* vma,
    vk::PhysicalDevice* physiscalDev,
    Device* dev, vk::Queue* transQueue,
    vk::CommandPool* transCmdPool,
    VulkanRenderer* vulkanRenderer)
{
    this->meshLoader.init(
        vma,
        physiscalDev,
        dev,
        transQueue,
        transCmdPool,
        this);
    this->textureLoader.init(
        vma,
        physiscalDev,
        dev,
        transQueue,
        transCmdPool,
        this);
    this->meshLoader.setTextureLoader(&this->textureLoader);
    this->textureLoader.setVulkanRenderer(vulkanRenderer);
    
}

uint32_t ResourceManager::addMesh(std::string&& meshPath)
{        
    using namespace vengine_helper::config;

    if(this->meshPaths.count(meshPath) != 0)
    {
        //TODO: should be able to log what mesh
        Log::warning("Mesh \""+meshPath+"\" was already added!");                
        return this->meshPaths[meshPath];        
    } 
    
    MeshData meshData = this->meshLoader.importMeshData(meshPath);
    // No mesh imported, send default mesh back
    if (meshData.vertices.size() == 0) { return 0; }

    //NOTE: prevSize as key only works if we never remove resources the map...
    this->meshPaths.insert({meshPath,this->meshPaths.size()}); 

    //NOTE: meshes.size as key only works if we never remove resources the map...    
    // Create mesh, insert into map of meshes
    meshes.insert({
        meshes.size(),
        meshLoader.createMesh(meshData)}        
        ); 

    return meshes.size() - 1;
}

uint32_t ResourceManager::addTexture(std::string&& texturePath)
{ 
    using namespace vengine_helper::config;

    if(this->texturePaths.count(texturePath) != 0)
    {
        Log::warning("Texture ["+texturePath+"] was already added!");                          
        return this->texturePaths[texturePath];
    }
    
    // Check if texture exists
    if (!this->textureLoader.doesTextureExist(texturePath))
    {
        Log::warning("Could not find texture: " + texturePath);
        return 0;
    }

    //NOTE: texturePaths.size() as key only works if we never remove resources the map...
    this->texturePaths.insert({texturePath,this->texturePaths.size()}); 

    //NOTE: textures.size as key only works if we never remove resources the map...    
    // Create texture, insert into map of textures
    textures.insert(
        {
            textures.size(), 
            textureLoader.createTexture(texturePath)
        }
    );

    return textures.size() -1;
}

void ResourceManager::cleanup()
{
    for(auto& elementPair : this->meshes)
    {                
        elementPair.second.cleanup();
    }

    for (auto& elementPair : this->textures)
    {     
        elementPair.second.cleanup();
    }
}