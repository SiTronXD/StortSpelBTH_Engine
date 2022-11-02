#include "ShaderInput.hpp"
#include "vulkan/PhysicalDevice.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/VulkanDbg.hpp"
#include "TextureSampler.hpp"
#include "../resource_management/ResourceManager.hpp"
#include "Texture.hpp"
#include "../dev/Log.hpp"

void ShaderInput::createDescriptorSetLayout()
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    // --------- Layout for per frame descriptor set layouts ---------
    std::vector<vk::DescriptorSetLayoutBinding> perFrameLayoutBindings;
    perFrameLayoutBindings.resize(this->addedUniformBuffers.size());
    for (size_t i = 0; i < perFrameLayoutBindings.size(); ++i)
    {
        perFrameLayoutBindings[i].setBinding(uint32_t(i));                                           // Describes which Binding Point in the shaders this layout is being bound to
        perFrameLayoutBindings[i].setDescriptorType(vk::DescriptorType::eUniformBuffer);    // Type of descriptor (Uniform, Dynamic uniform, image Sampler, etc.)
        perFrameLayoutBindings[i].setDescriptorCount(uint32_t(1));                                   // Amount of actual descriptors we're binding, where just binding one; our MVP struct
        perFrameLayoutBindings[i].setStageFlags(vk::ShaderStageFlagBits::eVertex);               // What Shader Stage we want to bind our Descriptor set to
        perFrameLayoutBindings[i].setPImmutableSamplers(nullptr);          // Used by Textures; whether or not the Sampler should be Immutable
    }
    
    vk::DescriptorSetLayoutCreateInfo perFrameLayoutCreateInfo{};
    perFrameLayoutCreateInfo.setBindingCount(
        static_cast<uint32_t>(perFrameLayoutBindings.size()));  // Number of Binding infos
    perFrameLayoutCreateInfo.setPBindings(
        perFrameLayoutBindings.data());                            // Array containing the binding infos

    // Create descriptor set layout
    this->perFrameSetLayout = this->device->getVkDevice().createDescriptorSetLayout(
        perFrameLayoutCreateInfo);
    VulkanDbg::registerVkObjectDbgInfo("DescriptorSetLayout ViewProjection", vk::ObjectType::eDescriptorSetLayout, reinterpret_cast<uint64_t>(vk::DescriptorSetLayout::CType(this->perFrameSetLayout)));

    // --------- Layout for per mesh descriptor set layouts ---------
    std::vector<vk::DescriptorSetLayoutBinding> perMeshLayoutBindings;
    perMeshLayoutBindings.resize(this->addedStorageBuffers.size());
    for (size_t i = 0; i < perMeshLayoutBindings.size(); ++i)
    {
        perMeshLayoutBindings[i].setBinding(uint32_t(i));                                           // Describes which Binding Point in the shaders this layout is being bound to
        perMeshLayoutBindings[i].setDescriptorType(vk::DescriptorType::eStorageBuffer);    // Type of descriptor (Uniform, Dynamic uniform, image Sampler, etc.)
        perMeshLayoutBindings[i].setDescriptorCount(uint32_t(1));                                   // Amount of actual descriptors we're binding, where just binding one; our MVP struct
        perMeshLayoutBindings[i].setStageFlags(vk::ShaderStageFlagBits::eVertex);               // What Shader Stage we want to bind our Descriptor set to
        perMeshLayoutBindings[i].setPImmutableSamplers(nullptr);          // Used by Textures; whether or not the Sampler should be Immutable
    }

    vk::DescriptorSetLayoutCreateInfo perMeshLayoutCreateInfo;
    perMeshLayoutCreateInfo.setBindingCount(
        static_cast<uint32_t>(perMeshLayoutBindings.size()));
    perMeshLayoutCreateInfo.setPBindings(
        perMeshLayoutBindings.data());

    // Create descriptor set layout
    this->perMeshSetLayout = 
        this->device->getVkDevice().createDescriptorSetLayout(
            perMeshLayoutCreateInfo);
    VulkanDbg::registerVkObjectDbgInfo("DescriptorSetLayout ViewProjection", vk::ObjectType::eDescriptorSetLayout, reinterpret_cast<uint64_t>(vk::DescriptorSetLayout::CType(this->perMeshSetLayout)));
    

    // --------- Layout for per draw descriptor set layouts ---------
    std::vector<vk::DescriptorSetLayoutBinding> perDrawLayoutBindings(
        this->samplersTextureIndex.size());
    for (size_t i = 0; i < perDrawLayoutBindings.size(); ++i)
    {
        perDrawLayoutBindings[i].setBinding(uint32_t(i));
        perDrawLayoutBindings[i].setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
        perDrawLayoutBindings[i].setDescriptorCount(uint32_t(1));
        perDrawLayoutBindings[i].setStageFlags(vk::ShaderStageFlagBits::eFragment);
        perDrawLayoutBindings[i].setPImmutableSamplers(nullptr);
    }

    vk::DescriptorSetLayoutCreateInfo perDrawLayoutCreateInfo;
    perDrawLayoutCreateInfo.setBindingCount(
        static_cast<uint32_t>(perDrawLayoutBindings.size()));
    perDrawLayoutCreateInfo.setPBindings(
        perDrawLayoutBindings.data());

    // Create sampler descriptor set layout
    this->perDrawSetLayout = this->device->getVkDevice().createDescriptorSetLayout(
        perDrawLayoutCreateInfo);
    VulkanDbg::registerVkObjectDbgInfo("DescriptorSetLayout SamplerTexture", vk::ObjectType::eDescriptorSetLayout, reinterpret_cast<uint64_t>(vk::DescriptorSetLayout::CType(this->perDrawSetLayout)));
}

void ShaderInput::createDescriptorPool()
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    if (this->addedUniformBuffers.size() > 0)
    {
        // --------- Descriptor pool for per frame descriptor sets ---------
        vk::DescriptorPoolSize perFramePoolSize{};
        perFramePoolSize.setType(vk::DescriptorType::eUniformBuffer);                                     // Descriptors in Set will be of Type Uniform Buffer
        perFramePoolSize.setDescriptorCount(
            this->framesInFlight * this->addedUniformBuffers.size());

        std::vector<vk::DescriptorPoolSize> perFramePoolSizes
        {
            perFramePoolSize
        };

        vk::DescriptorPoolCreateInfo perFramePoolCreateInfo{};
        perFramePoolCreateInfo.setMaxSets(
            this->framesInFlight * this->addedUniformBuffers.size());             // Max Nr Of descriptor Sets that can be created from the pool,
        perFramePoolCreateInfo.setPoolSizeCount(
            static_cast<uint32_t>(perFramePoolSizes.size()));   // Based on how many pools we have in our descriptorPoolSizes
        perFramePoolCreateInfo.setPPoolSizes(
            perFramePoolSizes.data());                        // PoolSizes to create the Descriptor Pool with

        // Create descriptor pool
        this->perFramePool = this->device->getVkDevice().createDescriptorPool(
            perFramePoolCreateInfo);
        VulkanDbg::registerVkObjectDbgInfo("DescriptorPool PerFrame", vk::ObjectType::eDescriptorPool, reinterpret_cast<uint64_t>(vk::DescriptorPool::CType(this->perFramePool)));
    }

    // --------- Descriptor pool for per mesh descriptor sets ---------
    if (this->addedStorageBuffers.size() > 0)
    {
        vk::DescriptorPoolSize perMeshPoolSize{};
        perMeshPoolSize.setType(vk::DescriptorType::eStorageBuffer);
        perMeshPoolSize.setDescriptorCount(
            this->framesInFlight * this->addedStorageBuffers.size());

        vk::DescriptorPoolCreateInfo perMeshPoolCreateInfo{};
        perMeshPoolCreateInfo.setMaxSets(perMeshPoolSize.descriptorCount);
        perMeshPoolCreateInfo.setPoolSizeCount(uint32_t(1));
        perMeshPoolCreateInfo.setPPoolSizes(&perMeshPoolSize);

        // Create descriptor pool
        this->perMeshPool =
            this->device->getVkDevice().createDescriptorPool(
                perMeshPoolCreateInfo);
        VulkanDbg::registerVkObjectDbgInfo("DescriptorPool PerMesh", vk::ObjectType::eDescriptorPool, reinterpret_cast<uint64_t>(vk::DescriptorPool::CType(this->perMeshPool)));
    }

    // --------- Descriptor pool for per draw descriptor sets ---------
    vk::DescriptorPoolSize perDrawPoolSize{};
    perDrawPoolSize.setType(vk::DescriptorType::eCombinedImageSampler);       // This descriptor pool will have descriptors for Image and Sampler combined    
    perDrawPoolSize.setDescriptorCount(MAX_NUM_TEXTURES);

    vk::DescriptorPoolCreateInfo samplerPoolCreateInfo{};
    samplerPoolCreateInfo.setMaxSets(MAX_NUM_TEXTURES);
    samplerPoolCreateInfo.setPoolSizeCount(uint32_t(1));
    samplerPoolCreateInfo.setPPoolSizes(&perDrawPoolSize);

    // Create descriptor pool
    this->perDrawPool = 
        this->device->getVkDevice().createDescriptorPool(
            samplerPoolCreateInfo);
    VulkanDbg::registerVkObjectDbgInfo("DescriptorPool PerDraw", vk::ObjectType::eDescriptorPool, reinterpret_cast<uint64_t>(vk::DescriptorPool::CType(this->perDrawPool)));
}

void ShaderInput::allocateDescriptorSets()
{
    // --------- Descriptor sets per frame ---------
    if (this->addedUniformBuffers.size() > 0)
    {
        // One descriptor set per frame in flight
        this->perFrameDescriptorSets.resize(this->framesInFlight);

        // Copy our layout so we have one per set
        std::vector<vk::DescriptorSetLayout> perFrameLayouts(
            this->perFrameDescriptorSets.size(),
            this->perFrameSetLayout
        );

        vk::DescriptorSetAllocateInfo perFrameAllocInfo;
        perFrameAllocInfo.setDescriptorPool(this->perFramePool);                                   // Pool to allocate descriptors (Set?) from   
        perFrameAllocInfo.setDescriptorSetCount(
            static_cast<uint32_t>(this->perFrameDescriptorSets.size()));
        perFrameAllocInfo.setPSetLayouts(perFrameLayouts.data());                               // Layouts to use to allocate sets (1:1 relationship)

        // Allocate descriptor sets
        this->perFrameDescriptorSets =
            this->device->getVkDevice().allocateDescriptorSets(
                perFrameAllocInfo);
    }

    // --------- Descriptor sets per mesh ---------
    if (this->addedStorageBuffers.size() > 0)
    {
        // One descriptor set per frame in flight per storage buffer
        uint32_t numStorageBuffers = this->addedStorageBuffers.size();
        this->perMeshDescriptorSets.resize(this->framesInFlight);
        for (size_t i = 0; i < this->perMeshDescriptorSets.size(); ++i)
        {
            this->perMeshDescriptorSets[i].resize(numStorageBuffers);

            // Copy our layout so we have one per set
            std::vector<vk::DescriptorSetLayout> perMeshLayouts(
                numStorageBuffers,
                this->perMeshSetLayout
            );

            vk::DescriptorSetAllocateInfo perMeshAllocInfo;
            perMeshAllocInfo.setDescriptorPool(this->perMeshPool);
            perMeshAllocInfo.setDescriptorSetCount(numStorageBuffers);
            perMeshAllocInfo.setPSetLayouts(perMeshLayouts.data());

            // Allocate descriptor sets
            this->perMeshDescriptorSets[i] =
                this->device->getVkDevice().allocateDescriptorSets(
                    perMeshAllocInfo);
        }
    }
}

void ShaderInput::updateDescriptorSets()
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    // --------- Per frame descriptor sets ---------

    // Update all per frame descriptor set buffer binding
    for (size_t i = 0; i < this->perFrameDescriptorSets.size(); i++)
    {
        std::vector<vk::DescriptorBufferInfo> descriptorBufferInfos;
        std::vector<vk::WriteDescriptorSet> writeDescriptorSets;
        descriptorBufferInfos.resize(this->addedUniformBuffers.size());
        writeDescriptorSets.resize(this->addedUniformBuffers.size());

        // Loop through all uniform buffers
        for (size_t j = 0; j < descriptorBufferInfos.size(); ++j)
        {
            // Describe the Buffer info and Data offset Info
            descriptorBufferInfos[j].setBuffer(
                this->addedUniformBuffers[j].getBuffer(i)); // Buffer to get the Data from
            descriptorBufferInfos[j].setOffset(
                0);
            descriptorBufferInfos[j].setRange(
                (vk::DeviceSize)this->addedUniformBuffers[j].getBufferSize());

            // Data to describe the connection between binding and uniform Buffer
            writeDescriptorSets[j].setDstSet(this->perFrameDescriptorSets[i]);              // Descriptor Set to update
            writeDescriptorSets[j].setDstBinding(uint32_t(j));                                    // Binding to update (Matches with Binding on Layout/Shader)
            writeDescriptorSets[j].setDstArrayElement(uint32_t(0));                                // Index in array we want to update (if we use an array, we do not. thus 0)
            writeDescriptorSets[j].setDescriptorType(vk::DescriptorType::eUniformBuffer);// Type of Descriptor
            writeDescriptorSets[j].setDescriptorCount(uint32_t(1));                                // Amount of Descriptors to update
            writeDescriptorSets[j].setPBufferInfo(&descriptorBufferInfos[j]);
        }

        // Update the descriptor sets with new buffer/binding info
        this->device->getVkDevice().updateDescriptorSets(
            writeDescriptorSets,
            nullptr
        );

        VulkanDbg::registerVkObjectDbgInfo("PerFrameDescriptorSet[" + std::to_string(i) + "]  UniformBuffer", vk::ObjectType::eDescriptorSet, reinterpret_cast<uint64_t>(vk::DescriptorSet::CType(this->perFrameDescriptorSets[i])));
    }


    // --------- Per mesh descriptor sets ---------
    for (size_t i = 0; i < this->perMeshDescriptorSets.size(); i++) 
    {
        std::vector<vk::DescriptorBufferInfo> descriptorBufferInfos;
        std::vector<vk::WriteDescriptorSet> writeDescriptorSets;
        descriptorBufferInfos.resize(this->addedStorageBuffers.size());
        writeDescriptorSets.resize(this->addedStorageBuffers.size());

        // Loop through all storage buffers
        for (size_t j = 0; j < descriptorBufferInfos.size(); ++j)
        {
            // Describe the Buffer info and Data offset Info
            descriptorBufferInfos[j].setBuffer(
                this->addedStorageBuffers[j].getBuffer(i)); // Buffer to get the Data from
            descriptorBufferInfos[j].setOffset(
                0);
            descriptorBufferInfos[j].setRange(
                (vk::DeviceSize)this->addedStorageBuffers[j].getBufferSize());

            // Data to describe the connection between binding and uniform Buffer
            writeDescriptorSets[j].setDstSet(this->perMeshDescriptorSets[i][j]);              // Descriptor Set to update
            writeDescriptorSets[j].setDstBinding(uint32_t(0));                                    // Binding to update (Matches with Binding on Layout/Shader)
            writeDescriptorSets[j].setDstArrayElement(uint32_t(0));                                // Index in array we want to update (if we use an array, we do not. thus 0)
            writeDescriptorSets[j].setDescriptorType(vk::DescriptorType::eStorageBuffer);// Type of Descriptor
            writeDescriptorSets[j].setDescriptorCount(uint32_t(1));                                // Amount of Descriptors to update
            writeDescriptorSets[j].setPBufferInfo(&descriptorBufferInfos[j]);

            VulkanDbg::registerVkObjectDbgInfo("PerMeshDescriptorSet[" + std::to_string(i) + "]  UniformBuffer", vk::ObjectType::eDescriptorSet, reinterpret_cast<uint64_t>(vk::DescriptorSet::CType(this->perMeshDescriptorSets[i][j])));
        }

        // Update the descriptor sets with new buffer/binding info
        this->device->getVkDevice().updateDescriptorSets(
            writeDescriptorSets,
            nullptr
        );
    }
}

int ShaderInput::addPossibleTexture(
    const uint32_t& textureIndex,
    TextureSampler& textureSampler)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    // Make sure the limit is not reached
    if (this->perDrawDescriptorSets.size() >= 
        MAX_NUM_TEXTURES)
    {
        Log::error("Reached maximum number of allowed textures for this shader.");

        return -1;
    }

    // Descriptor Set Allocation Info
    vk::DescriptorSetAllocateInfo setAllocateInfo;
    setAllocateInfo.setDescriptorPool(this->perDrawPool);
    setAllocateInfo.setDescriptorSetCount(uint32_t(1));
    setAllocateInfo.setPSetLayouts(&this->perDrawSetLayout);

    // Allocate Descriptor Sets
    vk::DescriptorSet descriptorSet =
        this->device->getVkDevice().allocateDescriptorSets(setAllocateInfo)[0];

    // Tedxture Image info
    vk::DescriptorImageInfo imageInfo;
    imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);     // The Image Layout when it is in use
    imageInfo.setImageView(
        this->resourceManager->getTexture(textureIndex).getImageView()); // Image to be bind to set
    imageInfo.setSampler(textureSampler.getVkSampler());                         // the Sampler to use for this Descriptor Set

    // Descriptor Write Info
    vk::WriteDescriptorSet writeDescriptorSet;
    writeDescriptorSet.setDstSet(descriptorSet);
    writeDescriptorSet.setDstBinding(0);
    writeDescriptorSet.setDstArrayElement(uint32_t(0));
    writeDescriptorSet.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
    writeDescriptorSet.setDescriptorCount(uint32_t(1));
    writeDescriptorSet.setPImageInfo(&imageInfo);

    // Update the new Descriptor Set
    this->device->getVkDevice().updateDescriptorSets(
        uint32_t(1),
        &writeDescriptorSet,
        uint32_t(0),
        nullptr
    );

    // Add descriptor Set to our list of descriptor Sets
    this->perDrawDescriptorSets.push_back(descriptorSet);

    // Return the last created Descriptor set
    return static_cast<int>(this->perDrawDescriptorSets.size() - 1);
}

ShaderInput::ShaderInput()
    : physicalDevice(nullptr),
    device(nullptr),
    vma(nullptr), 
    resourceManager(nullptr),
    framesInFlight(0),
    currentFrame(~0u),
    pushConstantSize(0),
    pushConstantShaderStage(vk::ShaderStageFlagBits::eAll), 
    usePushConstant(false),
    hasBeenCreated(false)
{ }

ShaderInput::~ShaderInput()
{
    
}

void ShaderInput::beginForInput(
    PhysicalDevice& physicalDevice,
    Device& device, 
    VmaAllocator& vma,
    ResourceManager& resourceManager,
    const uint32_t& framesInFlight)
{
    this->physicalDevice = &physicalDevice;
    this->device = &device;
    this->vma = &vma;
    this->resourceManager = &resourceManager;
    this->framesInFlight = framesInFlight;
}

UniformBufferID ShaderInput::addUniformBuffer(
    const size_t& contentsSize)
{
    UniformBufferID uniformBufferID = this->addedUniformBuffers.size();

    // Create and add uniform buffer
    this->addedUniformBuffers.push_back(UniformBuffer());
    this->addedUniformBuffers[uniformBufferID].createUniformBuffer(
        *this->device,
        *this->vma,
        contentsSize,
        this->framesInFlight
    );

    return uniformBufferID;
}

StorageBufferID ShaderInput::addStorageBuffer(
    const size_t& contentsSize) 
{
    StorageBufferID storageBufferID = this->addedStorageBuffers.size();

    // Create and add storage buffer
    this->addedStorageBuffers.push_back(StorageBuffer());
    this->addedStorageBuffers[storageBufferID].createStorageBuffer(
        *this->device,
        *this->vma,
        contentsSize,
        this->framesInFlight
    );

    return storageBufferID;
}

void ShaderInput::addPushConstant(
    const uint32_t& pushConstantSize,
    const vk::ShaderStageFlagBits& pushConstantShaderStage)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    this->pushConstantSize = pushConstantSize;
    this->pushConstantShaderStage = pushConstantShaderStage;
    this->usePushConstant = true;

    // Define the Push Constants values
    this->pushConstantRange.setStageFlags(this->pushConstantShaderStage);    // Push Constant should be available in the Vertex Shader!
    this->pushConstantRange.setOffset(uint32_t(0));                             // Offset into the given data that our Push Constant value is (??)
    this->pushConstantRange.setSize(this->pushConstantSize);                 // Size of the Data being passed
}

void ShaderInput::setNumShaderStorageBuffers(const uint32_t& numStorageBuffers)
{
    if (numStorageBuffers > 1)
    {
        Log::error("Multiple storage buffers are currently not supported. Ask an engine programmer for advice.");
        return;
    }
}

SamplerID ShaderInput::addSampler()
{
    if (this->samplersTextureIndex.size() > 0)
    {
        Log::error("Multiple samplers are currently not supported. Ask an engine programmer for advice.");
        return ~0u;
    }

    this->samplersTextureIndex.push_back(~0u);

    return this->samplersTextureIndex.size() - 1;
}

void ShaderInput::endForInput()
{
    this->createDescriptorSetLayout();
    this->createDescriptorPool();
    this->allocateDescriptorSets();
    this->updateDescriptorSets();

    // Descriptor set layouts to bind for pipeline layout
    this->bindDescriptorSetLayouts =
    {
        this->perFrameSetLayout,
        this->perMeshSetLayout,
        this->perDrawSetLayout
    };

    // Descriptor sets to bind when rendering
    this->bindDescriptorSets.resize(
        (int) DescriptorFrequency::NUM_FREQUENCY_TYPES);
    for (size_t i = 0; i < this->bindDescriptorSets.size(); ++i)
    {
        this->bindDescriptorSets[i] = nullptr;
    }

    // Pipeline layout
    this->pipelineLayout.createPipelineLayout(
        *this->device,
        *this
    );

    this->hasBeenCreated = true;
}

void ShaderInput::cleanup()
{
    // Don't cleanup if nothing has been created
    if (!this->hasBeenCreated)
        return;

    // Undo after destroying objects
    this->hasBeenCreated = false;

    // Uniform buffers
    for (size_t i = 0; i < this->addedUniformBuffers.size(); ++i)
    {
        this->addedUniformBuffers[i].cleanup();
    }
    this->addedUniformBuffers.clear();

    // Storage buffers
    for (size_t i = 0; i < this->addedStorageBuffers.size(); ++i)
    {
        this->addedStorageBuffers[i].cleanup();
    }
    this->addedStorageBuffers.clear();

    // Descriptor pools
    this->perFrameDescriptorSets.clear();
    this->perMeshDescriptorSets.clear();
    this->perDrawDescriptorSets.clear();
    this->device->getVkDevice().destroyDescriptorPool(this->perFramePool);
    this->device->getVkDevice().destroyDescriptorPool(this->perMeshPool);
    this->device->getVkDevice().destroyDescriptorPool(this->perDrawPool);

    // Descriptor set layouts
    this->device->getVkDevice().destroyDescriptorSetLayout(this->perFrameSetLayout);
    this->device->getVkDevice().destroyDescriptorSetLayout(this->perMeshSetLayout);
    this->device->getVkDevice().destroyDescriptorSetLayout(this->perDrawSetLayout);

    // Pipeline layout
    this->pipelineLayout.cleanup();

    // Sampler texture indices
    this->samplersTextureIndex.clear();

    // Vectors for binding during rendering
    this->bindDescriptorSetLayouts.clear();
    this->bindDescriptorSets.clear();
}

void ShaderInput::updateUniformBuffer(
    const UniformBufferID& id,
    void* data)
{
    this->addedUniformBuffers[id].update(data, this->currentFrame);
}

void ShaderInput::updateStorageBuffer(
    const StorageBufferID& id,
    void* data)
{
    this->addedStorageBuffers[id].update(data, this->currentFrame);
}

void ShaderInput::setCurrentFrame(const uint32_t& currentFrame)
{
    this->currentFrame = currentFrame;

    // Bind descriptor set
    if (this->perFrameDescriptorSets.size() > 0)
    {
        this->bindDescriptorSets[(uint32_t) DescriptorFrequency::PER_FRAME] =
            &this->perFrameDescriptorSets[currentFrame];
    }
}

void ShaderInput::setStorageBuffer(
    const UniformBufferID& uniformBufferID)
{
    this->bindDescriptorSets[(uint32_t)DescriptorFrequency::PER_MESH] =
        &this->perMeshDescriptorSets[this->currentFrame][uniformBufferID];
}

void ShaderInput::setTexture(
    const SamplerID& samplerID, 
    const uint32_t& textureIndex)
{
    // Current texture that sampler will use
    this->samplersTextureIndex[samplerID] = textureIndex;

    // Bind descriptor set
    this->bindDescriptorSets[(uint32_t) DescriptorFrequency::PER_DRAW_CALL] = 
        &this->perDrawDescriptorSets[textureIndex];
}