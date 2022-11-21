#include "pch.h"
#include "ResourceManager.hpp"
#include "Configurator.hpp"
#include "loaders/MeshLoader.hpp"
#include "loaders/TextureLoader.hpp"
#include "../graphics/MeshDataModifier.hpp"
#include "SFML/Audio/InputSoundFile.hpp"
#include "al.h"
#include "../VengineMath.hpp"

void ResourceManager::init(
    VmaAllocator* vma,
    PhysicalDevice* physicalDevice,
    Device* dev, vk::Queue* transQueue,
    vk::CommandPool* transCmdPool)
{
    this->dev = dev;

    this->meshLoader.init(
        vma,
        physicalDevice,
        dev,
        transQueue,
        transCmdPool,
        this);
    this->textureLoader.init(
        vma,
        physicalDevice,
        dev,
        transQueue,
        transCmdPool,
        this);
    this->meshLoader.setTextureLoader(&this->textureLoader);
    
}

void ResourceManager::makeUniqueMaterials(MeshComponent& meshComponent)
{
    const Mesh& mesh = this->getMesh(meshComponent.meshID);
    const std::vector<SubmeshData>& submeshData = mesh.getSubmeshData();

    // Copy over each material
    meshComponent.numOverrideMaterials = 0;
    for (size_t i = 0, numSubmeshes = submeshData.size(); i < numSubmeshes; ++i)
    {
        // Make sure the limit hasn't been reached
        if (meshComponent.numOverrideMaterials >= MAX_NUM_MESH_MATERIALS)
        {
            Log::warning("The number of materials was larger than the maximum. The remaining materials were ignored.");
            return;
        }

        // Copy material
        meshComponent.overrideMaterials[meshComponent.numOverrideMaterials++] =
            Material(this->getMaterial(submeshData[i].materialIndex));
    }
}

void ResourceManager::cleanUp()
{
    alDeleteBuffers((int)this->audioBuffers.size(), this->audioBuffers.data());
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
    const uint32_t meshId = (uint32_t)this->meshes.size();
    this->meshPaths.insert({meshPath, meshId}); 

    // Create mesh, insert into map of meshes
    this->meshes.insert({
        meshId,
        this->meshLoader.createMesh(meshData)}        
    ); 

    return meshes.size() - 1;
}

uint32_t ResourceManager::addMesh(std::string meshPath, MeshData meshData)
{
	// No mesh imported, send default mesh back
	if (meshData.vertexStreams.positions.size() == 0)
	{
		return 0;
	}

	// Smooth normals in mesh data
	// MeshDataModifier::smoothNormals(meshData);

	auto map_iterator = this->meshPaths.find(meshPath);
	if (this->meshPaths.end() == map_iterator)
	{
        //No mesh exists, create new
		this->meshPaths.insert({meshPath, this->meshes.size()});

		// NOTE: meshes.size as key only works if we never remove resources the map...
		// Create mesh, insert into map of meshes
		meshes.insert({meshes.size(), meshLoader.createMesh(meshData)});
	}
	else
	{
		uint32_t meshID = map_iterator->second;
        auto mesh_iterator = this->meshes.find(meshID);
        dev->waitIdle();
        mesh_iterator->second.cleanup();
        //deallocate memory, remove reference and create new mesh
		meshes.erase(meshID);
		meshes.insert({meshID, meshLoader.createMesh(meshData)});
    }
	

	return meshes.size() - 1;
}

uint32_t ResourceManager::addMaterial(std::string&& materialPath)
{
	using namespace vengine_helper::config;

	if (this->materialPaths.count(materialPath) != 0)
	{
		// Log::warning("Mesh \""+meshPath+"\" was already added!");

		return this->materialPaths[materialPath];
	} 

    Material mat = Material();

    this->materialPaths.insert({materialPath, this->materialPaths.size()}); 

    materials.insert({
        materials.size(), 
        mat}
    );

	return materials.size() -1;
}

uint32_t ResourceManager::addMaterial(std::string&& materialName, Material materialData)
{
	if (this->materialPaths.count(materialName) != 0)
	{
        //if already added, update data
        materials[materialPaths[materialName]] = materialData;
		return this->materialPaths[materialName];
	}

	this->materialPaths.insert({materialName, this->materials.size()});

	materials.insert({materials.size(), materialData});

	return materials.size() - 1;
}

uint32_t ResourceManager::addAnimations(const std::vector<std::string>& paths, std::string&& texturesPath)
{
    if (!paths.size())
    {
        Log::warning("Animation array contains no elements!");
        return 0;
    }

    for (const std::string& path : paths)
    {
        if (this->meshPaths.count(path) != 0)
        {
            // Log::warning("Mesh \""+meshPath+"\" was already added!");                

            return this->meshPaths[path];
        }
    }

    // load and store animations in meshData
    MeshData meshData;
    this->meshLoader.loadAnimations(paths, texturesPath, meshData);
    
    // No mesh imported, send default mesh back
    if (meshData.vertexStreams.positions.size() == 0) { return 0; }
    
    const uint32_t meshId = (uint32_t)meshes.size();
    this->meshes.insert({meshId, this->meshLoader.createMesh(meshData)});
    for (const std::string& path : paths)
    {
        this->meshPaths.insert({path, meshId});
    }

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

    uint32_t textureSamplerIndex = 
        this->addSampler(textureSettings);

    // NOTE: texturePaths.size() as key only works if we never remove resources the map...
    this->texturePaths.insert({texturePath,this->texturePaths.size()}); 

    // NOTE: textures.size as key only works if we never remove resources the map...    
    // Create texture, insert into map of textures
    textures.insert(
        {
            textures.size(), 
            textureLoader.createTexture(
                texturePath,
                textureSettings,
                textureSamplerIndex
            )
        }
    );

    return textures.size() - 1;
}

uint32_t ResourceManager::addSampler(
    const TextureSettings& textureSettings)
{
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

    return this->samplerSettings[samplerString];
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

uint32_t ResourceManager::addMaterial(
    uint32_t diffuseTextureIndex,
    uint32_t specularTextureIndex)
{
    // Created material
    Material newMaterial{};
    newMaterial.diffuseTextureIndex = diffuseTextureIndex;
    newMaterial.specularTextureIndex = specularTextureIndex;

    uint32_t newMaterialIndex =
        static_cast<uint32_t>(this->materials.size());

    // Insert material
    this->materials.insert({ newMaterialIndex, newMaterial });

    return newMaterialIndex;
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
        Log::warning("Failed opening audio file: " + soundPath);
        return 0;
    }

    // Allocate memory for samples
    const uint32_t sampleCount = (uint32_t)reader.getSampleCount();
    short* samples = new short[sampleCount]{};
    reader.read(samples, sampleCount);
    
    // Generate buffer
    uint32_t bufferId = -1;
    alGenBuffers(1, &bufferId);
    if (alGetError() != AL_NO_ERROR)
    {
        Log::warning("Failed generating audio buffer! File: " + soundPath);
        delete[]samples;
        return 0;
    }

    // Fill buffer
    const ALenum format = reader.getChannelCount() == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
    alBufferData(bufferId, format, samples, sizeof(short) * sampleCount, reader.getSampleRate());
    delete[]samples;
    if (alGetError() != AL_NO_ERROR)
    {
        Log::warning("Failed filling audio buffer! File: " + soundPath);
        return 0;
    }

    this->soundPaths.insert({soundPath, bufferId});
    this->audioBuffers.emplace_back(bufferId);

    return this->audioBuffers.back();
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

glm::mat4 ResourceManager::getJointTransform(
    Transform& animatedMeshTransform,
    MeshComponent& meshComp,
    AnimationComponent& animationComp,
    const std::string& boneName)
{
    // Find bone by name
    Mesh& mesh = this->getMesh(meshComp.meshID);
    MeshData& meshData = mesh.getMeshData();
    int32_t index = -1;
    for (size_t i = 0; i < meshData.bones.size(); ++i)
    {
        if (meshData.bones[i].boneName == boneName)
        {
            index = i;
            break;
        }
    }

    // Bone was not found
    if (index < 0)
    {
        Log::error("Could not find " + boneName + ".");
        return glm::mat4(1.0f);
    }

    animatedMeshTransform.updateMatrix();
    const glm::mat4& modelMat = animatedMeshTransform.getMatrix();

    // boneTransform = jointMatrix * inverseBindPoseMatrix
    // boneTransform * bindPoseMatrix = jointMatrix
    const glm::mat4 bindPoseMatrix = 
        glm::inverse(meshData.bones[index].inverseBindPoseMatrix);
    const glm::mat4& boneTransform =
        animationComp.getBoneTransformsData()[index];
    const glm::mat4 jointMatrix = boneTransform * bindPoseMatrix;

    // Composite
    glm::mat4 finalMatrix = 
        modelMat *
        jointMatrix;

    // The other mesh's scale should not affect this mesh
    SMath::normalizeScale(finalMatrix);

    return finalMatrix;
}