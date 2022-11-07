#include "pch.h"
#include "ResourceManager.hpp"
#include "Configurator.hpp"
#include "loaders/MeshLoader.hpp"
#include "loaders/TextureLoader.hpp"
#include "../graphics/MeshDataModifier.hpp"
#include "SFML/Audio/InputSoundFile.hpp"
#include "OpenAL/al.h"

void ResourceManager::init(
    VmaAllocator* vma,
    vk::PhysicalDevice* physiscalDev,
    Device* dev, vk::Queue* transQueue,
    vk::CommandPool* transCmdPool,
    VulkanRenderer* vulkanRenderer)
{
    this->dev = dev;

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

uint32_t ResourceManager::addMesh(
    std::string&& meshPath,
    std::string&& texturesPath)
{        
    using namespace vengine_helper::config;

    if (this->meshPaths.count(meshPath) != 0)
    {
        // Log::warning("Mesh \""+meshPath+"\" was already added!");                

        return this->meshPaths[meshPath];        
    } 
    
    MeshData meshData = this->meshLoader.importMeshData(
        meshPath, 
        texturesPath
    );

    // No mesh imported, send default mesh back
    if (meshData.vertexStreams.positions.size() == 0) { return 0; }

    // Smooth normals in mesh data
    // MeshDataModifier::smoothNormals(meshData);

    // NOTE: prevSize as key only works if we never remove resources the map...
    this->meshPaths.insert({meshPath,this->meshPaths.size()}); 

    // NOTE: meshes.size as key only works if we never remove resources the map...    
    // Create mesh, insert into map of meshes
    meshes.insert({
        meshes.size(),
        meshLoader.createMesh(meshData)}        
    ); 

    return meshes.size() - 1;
}

uint32_t ResourceManager::addAnimations(const std::vector<std::string>& paths, std::string&& texturesPath)
{
    if (!paths.size())
    {
        Log::warning("Animation array contains no elements!");
        return 0;
    }

    if (this->meshPaths.count(paths[0]) != 0)
    {
        // Log::warning("Mesh \""+meshPath+"\" was already added!");                

        return this->meshPaths[paths[0]];        
    }

    // load and store animations in meshData
    MeshData meshData;
    this->meshLoader.loadAnimations(paths, texturesPath, meshData);
    
    // No mesh imported, send default mesh back
    if (meshData.vertexStreams.positions.size() == 0) { return 0; }
    
    for (const std::string& path : paths)
    {
        this->meshPaths.insert({path, this->meshPaths.size()}); 
    }

    meshes.insert({(uint32_t)meshes.size(), meshLoader.createMesh(meshData)});
    return (uint32_t)meshes.size() - 1;
}

bool ResourceManager::mapAnimations(uint32_t meshid, const std::vector<std::string>& names)
{
    auto map_iterator = this->meshes.find(meshid);
    if (this->meshes.end() == map_iterator)
    {
        Log::error("mapAnimations failed to find a mesh with the given ID : "
            + std::to_string(meshid));
        return false;
    }
    this->getMesh(meshid).mapAnimations(names);

    return true;
}

uint32_t ResourceManager::addSound(std::string&& soundPath)
{
    if (this->soundPaths.count(soundPath) != 0)
    {
        return this->soundPaths[soundPath];   
    }

    sf::InputSoundFile reader;
    if (!reader.openFromFile(soundPath))
    {
        // reader outputs error string
        return 0;
    }

    // Allocate memory for samples
    const uint32_t sampleCount = (uint32_t)reader.getSampleCount();
    short* samples = new short[sampleCount]{};
    reader.read(samples, sampleCount);
    
    // Generate buffer
    AudioBufferId bufferId = ~1u;
    alGenBuffers(1, &bufferId);
    if (alGetError() != AL_NO_ERROR)
    {
        Log::warning("Failed creating audio buffer!");
        delete[]samples;
        return 0;
    }

    // Fill buffer
    const ALenum format = reader.getChannelCount() == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
    alBufferData(bufferId, format, samples, sizeof(short) * sampleCount, sampleCount);
    delete[]samples;
    if (alGetError() != AL_NO_ERROR)
    {
        Log::warning("Failed filling audio buffer!");
        return 0;
    }

    this->soundPaths.insert({soundPath, (uint32_t)this->soundPaths.size()});
    this->audioBuffers.insert({(uint32_t)this->audioBuffers.size(), bufferId});

    return (uint32_t)this->audioBuffers.size() - 1;
}

uint32_t ResourceManager::addTexture(
    std::string&& texturePath,
    const TextureSettings& textureSettings)
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

    // Create texture samples if it doesn't exist already
    std::string samplerString;
    TextureSampler::settingsToString(textureSettings.samplerSettings, samplerString);
    if (this->samplerSettings.count(samplerString) == 0)
    {
        // Add sampler index (indirection)
        this->samplerSettings.insert(
            {
                samplerString,
                this->textureSamplers.size()
            }
        );

        // Add texture sampler
        this->textureSamplers.insert(
            {
                this->samplerSettings[samplerString],
                TextureSampler()
            }
        );

        // Create sampler
        this->textureSamplers[this->samplerSettings[samplerString]].
            createSampler(*this->dev, textureSettings.samplerSettings);
    }

    //NOTE: texturePaths.size() as key only works if we never remove resources the map...
    this->texturePaths.insert({texturePath,this->texturePaths.size()}); 

    //NOTE: textures.size as key only works if we never remove resources the map...    
    // Create texture, insert into map of textures
    textures.insert(
        {
            textures.size(), 
            textureLoader.createTexture(
                texturePath,
                textureSettings,
                this->samplerSettings[samplerString]
            )
        }
    );

    return textures.size() - 1;
}

uint32_t ResourceManager::addCollisionShapeFromMesh(std::string&& collisionPath)
{
	using namespace vengine_helper::config;

	if (this->collisionPaths.count(collisionPath) != 0)
	{
		//TODO: should be able to log what mesh
		Log::warning("Collision \"" + collisionPath + "\" was already added!");
		return this->collisionPaths[collisionPath];
	}
    
	std::vector<ColliderDataRes> collisions = collisionLoader.loadCollisionShape(collisionPath);
	//NOTE: prevSize as key only works if we never remove resources the map...
	 this->collisionPaths.insert({collisionPath, this->collisionPaths.size()});
    
	//NOTE: meshes.size as key only works if we never remove resources the map...
	// Create mesh, insert into map of meshes
	 collisionsData.insert({collisionsData.size(), collisions});

	return collisionsData.size() - 1;
}

void ResourceManager::cleanup()
{
    // Meshes
    for(auto& elementPair : this->meshes)
    {                
        elementPair.second.cleanup();
    }

    // Textures
    for (auto& elementPair : this->textures)
    {     
        elementPair.second.cleanup();
    }

    // Texture samplers
    for (auto& elementPair : this->textureSamplers)
    {
        elementPair.second.cleanup();
    }
}