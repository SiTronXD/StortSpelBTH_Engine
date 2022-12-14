#include "pch.h"
#include "ShaderInput.hpp"
#include "vulkan/PhysicalDevice.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/VulkanDbg.hpp"
#include "TextureSampler.hpp"
#include "../resource_management/ResourceManager.hpp"
#include "Texture.hpp"
#include "../dev/Log.hpp"

uint64_t ShaderInput::createResourceID(
    const DescriptorFrequency& descriptorFrequency)
{
    uint32_t frequencyIndex = (uint32_t)descriptorFrequency;
    uint64_t resourceIndex = 
        static_cast<uint64_t>(this->resources[frequencyIndex].size());

    return (uint64_t(frequencyIndex) << 32) | resourceIndex;
}

uint32_t ShaderInput::getResourceFrequencyIndex(
    const uint64_t& resourceID)
{
    return uint32_t(resourceID >> 32);
}

uint32_t ShaderInput::getResourceIndex(
    const uint64_t& resourceID)
{
    return uint32_t(resourceID);
}

void ShaderInput::createDescriptorSetLayouts()
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    // layoutBindings[DescriptorFrequency][bindingIndex]
    std::vector<std::vector<vk::DescriptorSetLayoutBinding>> layoutBindings((uint32_t) DescriptorFrequency::NUM_FREQUENCY_TYPES);

    // Resources
    for (size_t i = 0; i < layoutBindings.size(); ++i)
    {
        for (size_t j = 0; j < this->resources[i].size(); ++j)
        {
            const ResourceHandle& resource = this->resources[i][j];

            vk::DescriptorSetLayoutBinding newLayoutBinding{};
            newLayoutBinding.setBinding(uint32_t(j));                                           // Describes which Binding Point in the shaders this layout is being bound to
            newLayoutBinding.setDescriptorType(resource.descriptorType);    // Type of descriptor (Uniform, Dynamic uniform, image Sampler, etc.)
            newLayoutBinding.setDescriptorCount(uint32_t(1));                                   // Amount of actual descriptors we're binding, where just binding one; our MVP struct
            newLayoutBinding.setStageFlags(resource.shaderStage);               // What Shader Stage we want to bind our Descriptor set to
            newLayoutBinding.setPImmutableSamplers(nullptr);          // Used by Textures; whether or not the Sampler should be Immutable
        
            layoutBindings[i].push_back(newLayoutBinding);
        }
    }

    // Combined image/samplers per draw
    for (size_t i = 0; i < this->perDrawInputLayout.numBindings; ++i)
    {
        vk::DescriptorSetLayoutBinding combSampLayoutBinding{};
        combSampLayoutBinding.setBinding(uint32_t(i));
        combSampLayoutBinding.setDescriptorType(this->perDrawInputLayout.descriptorBindingsTypes[i]);
        combSampLayoutBinding.setDescriptorCount(uint32_t(1));
        combSampLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eFragment);
        combSampLayoutBinding.setPImmutableSamplers(nullptr);

        // Always per draw for now
        layoutBindings[(uint32_t) DescriptorFrequency::PER_DRAW_CALL].push_back(combSampLayoutBinding);
    }


    // Per frame
    vk::DescriptorSetLayoutCreateInfo perFrameLayoutCreateInfo{};
    perFrameLayoutCreateInfo.setBindingCount(
        static_cast<uint32_t>(layoutBindings[(uint32_t)DescriptorFrequency::PER_FRAME].size()));  // Number of Binding infos
    perFrameLayoutCreateInfo.setPBindings(
        layoutBindings[(uint32_t)DescriptorFrequency::PER_FRAME].data());                            // Array containing the binding infos

    // --------- Create descriptor set layout for per frame descriptor sets ---------
    this->perFrameSetLayout = this->device->getVkDevice().createDescriptorSetLayout(
        perFrameLayoutCreateInfo);
    VulkanDbg::registerVkObjectDbgInfo("PerFrameDescriptorSetLayout", vk::ObjectType::eDescriptorSetLayout, reinterpret_cast<uint64_t>(vk::DescriptorSetLayout::CType(this->perFrameSetLayout)));

    // Per mesh
    vk::DescriptorSetLayoutCreateInfo perMeshLayoutCreateInfo;
    perMeshLayoutCreateInfo.setBindingCount(
        static_cast<uint32_t>(layoutBindings[(uint32_t)DescriptorFrequency::PER_MESH].size()));
    perMeshLayoutCreateInfo.setPBindings(
        layoutBindings[(uint32_t)DescriptorFrequency::PER_MESH].data());

    // --------- Create descriptor set layout for per mesh descriptor sets ---------
    this->perMeshSetLayout =
        this->device->getVkDevice().createDescriptorSetLayout(
            perMeshLayoutCreateInfo);
    VulkanDbg::registerVkObjectDbgInfo("PerMeshDescriptorSetLayout", vk::ObjectType::eDescriptorSetLayout, reinterpret_cast<uint64_t>(vk::DescriptorSetLayout::CType(this->perMeshSetLayout)));

    // Per draw
    vk::DescriptorSetLayoutCreateInfo perDrawLayoutCreateInfo;
    perDrawLayoutCreateInfo.setBindingCount(
        static_cast<uint32_t>(layoutBindings[(uint32_t)DescriptorFrequency::PER_DRAW_CALL].size()));
    perDrawLayoutCreateInfo.setPBindings(
        layoutBindings[(uint32_t)DescriptorFrequency::PER_DRAW_CALL].data());

    // --------- Create descriptor set layout for per draw descriptor sets ---------
    this->perDrawSetLayout = this->device->getVkDevice().createDescriptorSetLayout(
        perDrawLayoutCreateInfo);
    VulkanDbg::registerVkObjectDbgInfo("PerDrawDescriptorSetLayout", vk::ObjectType::eDescriptorSetLayout, reinterpret_cast<uint64_t>(vk::DescriptorSetLayout::CType(this->perDrawSetLayout)));
}

void ShaderInput::createDescriptorPools()
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    enum DescriptorTypes
    {
        UNIFORM_BUFFER,
        STORAGE_BUFFER,
        COMBINED_IMAGE_SAMPLER,

        NUM_DESC_TYPES
    };

    // descriptorToVkDescriptor[i] = vk::DescriptorType
    vk::DescriptorType descriptorToVkDescriptor[3]
    {
        vk::DescriptorType::eUniformBuffer,
        vk::DescriptorType::eStorageBuffer,
        vk::DescriptorType::eCombinedImageSampler
    };

    // vkDescriptorToDescriptor[vk::DesciptorType] = i
    // (vk::DesciptorType can be a very large number)
    std::map<vk::DescriptorType, uint32_t> vkDescriptorToDescriptor;
    for (uint32_t i = 0; i < DescriptorTypes::NUM_DESC_TYPES; ++i)
    {
        vkDescriptorToDescriptor.insert(
            {
                descriptorToVkDescriptor[i],
                i
            }
        );
    }

    // Pool sizes
    // poolSizes[descriptorSetFrequency][descriptorType]
    std::vector<std::vector<vk::DescriptorPoolSize>> poolSizes((uint32_t) DescriptorFrequency::NUM_FREQUENCY_TYPES);
    for (size_t i = 0; i < poolSizes.size(); ++i)
    {
        for (size_t j = 0; j < DescriptorTypes::NUM_DESC_TYPES; ++j)
        {
            vk::DescriptorPoolSize descPoolSize{};
            descPoolSize.setType(descriptorToVkDescriptor[j]);
            descPoolSize.setDescriptorCount(0);

            poolSizes[i].push_back(descPoolSize);
        }
    }

    // Find number of descriptor sets to create
    for (size_t i = 0; i < this->resources.size(); ++i)
    {
        for (size_t j = 0; j < this->resources[i].size(); ++j)
        {
            const ResourceHandle& resource = this->resources[i][j];

            poolSizes
                [(uint32_t) resource.descriptorFreq]
                [vkDescriptorToDescriptor[resource.descriptorType]].descriptorCount 
                    += resource.cpuWritable ?
                    this->framesInFlight : 1;
        }
    }


    // --------- Descriptor pool for per frame descriptor sets ---------
    std::vector<vk::DescriptorPoolSize>& perFramePoolSizes = poolSizes[(uint32_t) DescriptorFrequency::PER_FRAME];
    uint32_t perFrameDescriptorCount = 0;
    for(size_t i = 0; i < perFramePoolSizes.size(); ++i)
    {
        if (perFramePoolSizes[i].descriptorCount <= 0)
        {
            perFramePoolSizes.erase(perFramePoolSizes.begin() + i);
            i--;
            continue;
        }

        perFrameDescriptorCount += perFramePoolSizes[i].descriptorCount;
    }
    if (perFrameDescriptorCount > 0)
    {
        // Pool create info
        vk::DescriptorPoolCreateInfo perFramePoolCreateInfo{};
        perFramePoolCreateInfo.setMaxSets(perFrameDescriptorCount);             // Max Nr Of descriptor Sets that can be created from the pool,
        perFramePoolCreateInfo.setPoolSizeCount(
            static_cast<uint32_t>(perFramePoolSizes.size()));   // Based on how many pools we have in our descriptorPoolSizes
        perFramePoolCreateInfo.setPPoolSizes(
            perFramePoolSizes.data());                        // PoolSizes to create the Descriptor Pool with

        // Create per frame descriptor pool
        this->perFramePool =
            this->device->getVkDevice().createDescriptorPool(
                perFramePoolCreateInfo);
        VulkanDbg::registerVkObjectDbgInfo("PerFrameDescriptorPool", vk::ObjectType::eDescriptorPool, reinterpret_cast<uint64_t>(vk::DescriptorPool::CType(this->perFramePool)));
    }


    // --------- Descriptor pool for per mesh descriptor sets ---------
    std::vector<vk::DescriptorPoolSize>& perMeshPoolSizes = poolSizes[(uint32_t) DescriptorFrequency::PER_MESH];
    uint32_t perMeshDescriptorCount = 0;
    for (size_t i = 0; i < perMeshPoolSizes.size(); ++i)
    {
        if (perMeshPoolSizes[i].descriptorCount <= 0)
        {
            perMeshPoolSizes.erase(perMeshPoolSizes.begin() + i);
            i--;
            continue;
        }

        perMeshDescriptorCount += perMeshPoolSizes[i].descriptorCount;
    }
    if (perMeshDescriptorCount > 0)
    {
        // Pool create info 
        vk::DescriptorPoolCreateInfo perMeshPoolCreateInfo{};
        perMeshPoolCreateInfo.setMaxSets(perMeshDescriptorCount);
        perMeshPoolCreateInfo.setPoolSizeCount(
            static_cast<uint32_t>(perMeshPoolSizes.size()));
        perMeshPoolCreateInfo.setPPoolSizes(perMeshPoolSizes.data());

        // Create per mesh descriptor pool
        this->perMeshPool =
            this->device->getVkDevice().createDescriptorPool(
                perMeshPoolCreateInfo);
        VulkanDbg::registerVkObjectDbgInfo("PerMeshDescriptorPool", vk::ObjectType::eDescriptorPool, reinterpret_cast<uint64_t>(vk::DescriptorPool::CType(this->perMeshPool)));
    }


    // --------- Descriptor pool for per draw descriptor sets ---------
    vk::DescriptorPoolSize perDrawPoolSize{};
    perDrawPoolSize.setType(vk::DescriptorType::eCombinedImageSampler);       // This descriptor pool will have descriptors for Image and Sampler combined    
    perDrawPoolSize.setDescriptorCount(MAX_NUM_PER_DRAW_DESCRIPTOR_SETS);

    vk::DescriptorPoolCreateInfo samplerPoolCreateInfo{};
    samplerPoolCreateInfo.setMaxSets(perDrawPoolSize.descriptorCount);
    samplerPoolCreateInfo.setPoolSizeCount(uint32_t(1));
    samplerPoolCreateInfo.setPPoolSizes(&perDrawPoolSize);

    // Create combined image sampler descriptor pool 
    this->perDrawPool = 
        this->device->getVkDevice().createDescriptorPool(
            samplerPoolCreateInfo);
    VulkanDbg::registerVkObjectDbgInfo("DescriptorPool CombSamp", vk::ObjectType::eDescriptorPool, reinterpret_cast<uint64_t>(vk::DescriptorPool::CType(this->perDrawPool)));
}

void ShaderInput::allocateDescriptorSets()
{
    // --------- Descriptor sets per frame ---------
    const std::vector<ResourceHandle>& perFrameResources =
        this->resources[(uint32_t)DescriptorFrequency::PER_FRAME];
    if (perFrameResources.size() > 0)
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
    const std::vector<ResourceHandle>& perMeshResources =
        this->resources[(uint32_t)DescriptorFrequency::PER_MESH];
    if (perMeshResources.size() > 0)
    {
        // One descriptor set per frame in flight per storage buffer
        this->perMeshDescriptorSets.resize(this->framesInFlight);
        for (size_t i = 0; i < this->perMeshDescriptorSets.size(); ++i)
        {
            this->perMeshDescriptorSets[i].resize(perMeshResources.size());

            // Copy our layout so we have one per set
            std::vector<vk::DescriptorSetLayout> perMeshLayouts(
                this->perMeshDescriptorSets[i].size(),
                this->perMeshSetLayout
            );

            vk::DescriptorSetAllocateInfo perMeshAllocInfo;
            perMeshAllocInfo.setDescriptorPool(this->perMeshPool);
            perMeshAllocInfo.setDescriptorSetCount(this->perMeshDescriptorSets[i].size());
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
    const std::vector<ResourceHandle>& perFrameResources =
        this->resources[(uint32_t) DescriptorFrequency::PER_FRAME];
    for (size_t i = 0; i < this->perFrameDescriptorSets.size(); i++)
    {
        std::vector<vk::DescriptorBufferInfo> descriptorBufferInfos;
        std::vector<vk::DescriptorImageInfo> descriptorImageInfos;
        std::vector<vk::WriteDescriptorSet> writeDescriptorSets;
        descriptorBufferInfos.resize(perFrameResources.size());
        descriptorImageInfos.resize(perFrameResources.size());
        writeDescriptorSets.resize(perFrameResources.size());

        // Loop through all uniform buffers
        for (size_t j = 0; j < descriptorBufferInfos.size(); ++j)
        {
            Buffer* buffer = perFrameResources[j].buffer;
            Texture* texture = perFrameResources[j].textureReference;

            // Buffer
            if (buffer != nullptr)
            {
                // Describe the Buffer info and Data offset Info
                descriptorBufferInfos[j].setBuffer(
                    buffer->getBuffer(i)); // Buffer to get the Data from
                descriptorBufferInfos[j].setOffset(
                    0);
                descriptorBufferInfos[j].setRange(
                    (vk::DeviceSize)buffer->getBufferSize());

                writeDescriptorSets[j].setPBufferInfo(&descriptorBufferInfos[j]);
            }
            // Texture
            else
            {
                descriptorImageInfos[j].setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal); // The image layout when it is in use
                descriptorImageInfos[j].setImageView(texture->getImageView());
                descriptorImageInfos[j].setSampler(
                    this->resourceManager->getTextureSampler(
                        texture->getSamplerIndex()
                    ).getVkSampler()
                );

                writeDescriptorSets[j].setPImageInfo(&descriptorImageInfos[j]);
            }

            // Data to describe the connection between binding and uniform Buffer
            writeDescriptorSets[j].setDstSet(this->perFrameDescriptorSets[i]);              // Descriptor Set to update
            writeDescriptorSets[j].setDstBinding(uint32_t(j));                                    // Binding to update (Matches with Binding on Layout/Shader)
            writeDescriptorSets[j].setDstArrayElement(uint32_t(0));                                // Index in array we want to update (if we use an array, we do not. thus 0)
            writeDescriptorSets[j].setDescriptorType(perFrameResources[j].descriptorType);// Type of Descriptor
            writeDescriptorSets[j].setDescriptorCount(uint32_t(1));                                // Amount of Descriptors to update
        }

        // Update the descriptor sets with new buffer/binding info
        this->device->getVkDevice().updateDescriptorSets(
            writeDescriptorSets,
            nullptr
        );

        VulkanDbg::registerVkObjectDbgInfo("PerFrameDescriptorSet[" + std::to_string(i) + "]", vk::ObjectType::eDescriptorSet, reinterpret_cast<uint64_t>(vk::DescriptorSet::CType(this->perFrameDescriptorSets[i])));
    }

    // --------- Per mesh descriptor sets ---------
    const std::vector<ResourceHandle>& perMeshResources =
        this->resources[(uint32_t)DescriptorFrequency::PER_MESH];
    for (size_t i = 0; i < this->perMeshDescriptorSets.size(); i++) 
    {
        std::vector<vk::DescriptorBufferInfo> descriptorBufferInfos;
        std::vector<vk::WriteDescriptorSet> writeDescriptorSets;
        descriptorBufferInfos.resize(perMeshResources.size());
        writeDescriptorSets.resize(perMeshResources.size());

        // Loop through all storage buffers
        for (size_t j = 0; j < descriptorBufferInfos.size(); ++j)
        {
            Buffer* buffer = perMeshResources[j].buffer;

            // Describe the Buffer info and Data offset Info
            descriptorBufferInfos[j].setBuffer(
                buffer->getBuffer(i)); // Buffer to get the Data from
            descriptorBufferInfos[j].setOffset(
                0);
            descriptorBufferInfos[j].setRange(
                (vk::DeviceSize) buffer->getBufferSize());

            // Data to describe the connection between binding and uniform Buffer
            writeDescriptorSets[j].setDstSet(this->perMeshDescriptorSets[i][j]);              // Descriptor Set to update
            writeDescriptorSets[j].setDstBinding(0 /* 0 for animations */);                                    // Binding to update (Matches with Binding on Layout/Shader)
            writeDescriptorSets[j].setDstArrayElement(uint32_t(0));                                // Index in array we want to update (if we use an array, we do not. thus 0)
            writeDescriptorSets[j].setDescriptorType(perMeshResources[j].descriptorType);// Type of Descriptor
            writeDescriptorSets[j].setDescriptorCount(uint32_t(1));                                // Amount of Descriptors to update
            writeDescriptorSets[j].setPBufferInfo(&descriptorBufferInfos[j]);

            VulkanDbg::registerVkObjectDbgInfo("PerMeshDescriptorSet[" + std::to_string(i) + "]", vk::ObjectType::eDescriptorSet, reinterpret_cast<uint64_t>(vk::DescriptorSet::CType(this->perMeshDescriptorSets[i][j])));
        }

        // Update the descriptor sets with new buffer/binding info
        this->device->getVkDevice().updateDescriptorSets(
            writeDescriptorSets,
            nullptr
        );
    }
}

ShaderInput::ShaderInput()
    : physicalDevice(nullptr),
    device(nullptr),
    vma(nullptr),
    transferQueue(nullptr),
    transferCommandPool(nullptr),
    resourceManager(nullptr),
    framesInFlight(0),
    currentFrame(~0u),
    pushConstantSize(0),
    pushConstantShaderStage(vk::ShaderStageFlagBits::eAll), 
    usePushConstant(false),
    hasBeenCreated(false)
{ 
    this->resources.resize((uint32_t) DescriptorFrequency::NUM_FREQUENCY_TYPES);
}

ShaderInput::~ShaderInput()
{
    
}

void ShaderInput::initForGpuOnlyResources(
    vk::Queue& transferQueue,
    vk::CommandPool& transferCommandPool)
{
    this->transferQueue = &transferQueue;
    this->transferCommandPool = &transferCommandPool;
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
    const size_t& contentsSize,
    const vk::ShaderStageFlagBits& shaderStage,
    const DescriptorFrequency& descriptorFrequency)
{
    UniformBufferID uniformBufferID = 
        this->createResourceID(descriptorFrequency);

    // Create uniform buffer
    UniformBuffer* uniformBuffer = new UniformBuffer();
    uniformBuffer->createUniformBuffer(
        *this->device,
        *this->vma,
        contentsSize,
        this->framesInFlight
    );

    // Create resource handle and add it to the lists
    ResourceHandle resourceHandle{};
    resourceHandle.buffer = uniformBuffer;
    resourceHandle.descriptorType = vk::DescriptorType::eUniformBuffer;
    resourceHandle.shaderStage = shaderStage;
    resourceHandle.descriptorFreq = descriptorFrequency;
    resourceHandle.cpuWritable = true;

    // Add resource to list
    this->resources[(uint32_t)descriptorFrequency].push_back(resourceHandle);

    return uniformBufferID;
}

StorageBufferID ShaderInput::addStorageBuffer(
    const size_t& contentsSize,
    const vk::ShaderStageFlagBits& shaderStage,
    const DescriptorFrequency& descriptorFrequency,
    const bool& gpuOnly,
    void* initialData)
{
    StorageBufferID storageBufferID = 
        this->createResourceID(descriptorFrequency);

    // Create storage buffer
    StorageBuffer* storageBuffer = new StorageBuffer();
    storageBuffer->createStorageBuffer(
        *this->device,
        *this->vma,
        contentsSize,
        this->framesInFlight,
        gpuOnly,
        initialData,
        this->transferQueue,
        this->transferCommandPool
    );

    // Create resource handle and add it to the lists
    ResourceHandle resourceHandle{};
    resourceHandle.buffer = storageBuffer;
    resourceHandle.descriptorType = vk::DescriptorType::eStorageBuffer;
    resourceHandle.shaderStage = shaderStage;
    resourceHandle.descriptorFreq = descriptorFrequency;
    resourceHandle.cpuWritable = true;

    // Add resource to list
    this->resources[(uint32_t) descriptorFrequency].push_back(resourceHandle);

    return storageBufferID;
}

void ShaderInput::addCombinedImageSampler(
    Texture& texture,
    const vk::ShaderStageFlagBits& shaderStage,
    const DescriptorFrequency& descriptorFrequency)
{
    // Create resource handle and add it to the lists
    ResourceHandle resourceHandle{};
    resourceHandle.buffer = nullptr;
    resourceHandle.textureReference = &texture;
    resourceHandle.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    resourceHandle.shaderStage = shaderStage;
    resourceHandle.descriptorFreq = descriptorFrequency;
    resourceHandle.cpuWritable = true;

    // Add resource to list
    this->resources[(uint32_t)descriptorFrequency].push_back(resourceHandle);
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

void ShaderInput::endForInput()
{
    this->createDescriptorSetLayouts();
    this->createDescriptorPools();
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

    // Resource handles
    for (size_t i = 0; 
        i < (size_t) DescriptorFrequency::NUM_FREQUENCY_TYPES; 
        ++i)
    {
        // Resources
        for (size_t j = 0, numResources = this->resources[i].size();
            j < numResources;
            ++j)
        {
            Buffer*& buffer =
                this->resources[i][j].buffer;

            // Only delete buffers
            if (buffer != nullptr)
            {
                buffer->cleanup();

                delete buffer;
                buffer = nullptr;
            }
        }

        this->resources[i].clear();
    }

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

    // Vectors for binding during rendering
    this->bindDescriptorSetLayouts.clear();
    this->bindDescriptorSets.clear();
}

void ShaderInput::updateUniformBuffer(
    const UniformBufferID& id,
    void* data)
{
    this->resources
        [this->getResourceFrequencyIndex(id)]
        [this->getResourceIndex(id)]
        .buffer->update(data, this->currentFrame);
}

void ShaderInput::updateStorageBuffer(
    const StorageBufferID& id,
    void* data)
{
    this->resources
        [this->getResourceFrequencyIndex(id)]
        [this->getResourceIndex(id)]
        .buffer->update(data, this->currentFrame);
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
    const StorageBufferID& storageBufferID)
{
    // TODO: make this prettier to look at
    this->bindDescriptorSets
        [this->getResourceFrequencyIndex(storageBufferID)] =
            &this->perMeshDescriptorSets
            [this->currentFrame]
            [this->getResourceIndex(storageBufferID)];
}

void ShaderInput::makeFrequencyInputLayout(
    // const DescriptorFrequency& descriptorFrequency,
    const FrequencyInputLayout& bindingsLayout)
{
    this->perDrawInputLayout = bindingsLayout;
}

uint32_t ShaderInput::addFrequencyInput(
    const std::vector<FrequencyInputBindings>& bindings)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    if (bindings.size() != this->perDrawInputLayout.numBindings)
    {
        Log::error("The number of bindings (" + std::to_string(this->perDrawInputLayout.numBindings) + ") does not match the provided layout (" + std::to_string(bindings.size()) + ").");
        return ~0u;
    }

    // Make sure the limit is not reached
    if (this->perDrawDescriptorSets.size() >=
        MAX_NUM_PER_DRAW_DESCRIPTOR_SETS)
    {
        Log::error("Reached maximum number of allowed descriptor sets for this shader.");

        return -1;
    }

    // Descriptor set allocation info
    vk::DescriptorSetAllocateInfo setAllocateInfo;
    setAllocateInfo.setDescriptorPool(this->perDrawPool);
    setAllocateInfo.setDescriptorSetCount(uint32_t(1));
    setAllocateInfo.setPSetLayouts(&this->perDrawSetLayout);

    // Allocate descriptor sets
    vk::DescriptorSet descriptorSet =
        this->device->getVkDevice().allocateDescriptorSets(setAllocateInfo)[0];
    std::vector<vk::DescriptorImageInfo> descriptorImageInfos(bindings.size());
    std::vector<vk::WriteDescriptorSet> writeDescriptorSets(bindings.size());
    for (size_t i = 0; i < writeDescriptorSets.size(); ++i)
    {
        // Texture image info
        descriptorImageInfos[i].setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);     // The Image Layout when it is in use
        descriptorImageInfos[i].setImageView(
            bindings[i].imageView == nullptr ? 
            bindings[i].texture->getImageView() :
            *bindings[i].imageView); // Image to be bind to set
        descriptorImageInfos[i].setSampler(
            this->resourceManager->getTextureSampler(
                bindings[i].texture->getSamplerIndex()
            ).getVkSampler()
        );                         // The sampler to use for this descriptor set

        // Descriptor write info
        writeDescriptorSets[i].setDstSet(descriptorSet);
        writeDescriptorSets[i].setDstBinding(uint32_t(i));
        writeDescriptorSets[i].setDstArrayElement(uint32_t(0));
        writeDescriptorSets[i].setDescriptorType(this->perDrawInputLayout.descriptorBindingsTypes[i]);
        writeDescriptorSets[i].setDescriptorCount(uint32_t(1)); // (Num pImageInfos)
        writeDescriptorSets[i].setPImageInfo(&descriptorImageInfos[i]); 
    }

    // Update descriptor sets
    this->device->getVkDevice().updateDescriptorSets(
        writeDescriptorSets,
        nullptr
    );

    // Add descriptor set to our list of descriptor sets
    this->perDrawDescriptorSets.push_back(descriptorSet);

    // Return the last created descriptor set
    return static_cast<int>(this->perDrawDescriptorSets.size() - 1);
}

void ShaderInput::updateFrequencyInput(
    const std::vector<FrequencyInputBindings>& bindings,
    const uint32_t& descriptorIndex)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    if (bindings.size() != this->perDrawInputLayout.numBindings)
    {
        Log::error("The number of bindings (" + std::to_string(this->perDrawInputLayout.numBindings) + ") does not match the provided layout (" + std::to_string(bindings.size()) + ").");
        return;
    }

    // Get descriptor set from list
    vk::DescriptorSet& descriptorSet = 
        this->perDrawDescriptorSets[descriptorIndex];

    std::vector<vk::DescriptorImageInfo> descriptorImageInfos(bindings.size());
    std::vector<vk::WriteDescriptorSet> writeDescriptorSets(bindings.size());
    for (size_t i = 0; i < writeDescriptorSets.size(); ++i)
    {
        // Texture image info
        descriptorImageInfos[i].setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);     // The Image Layout when it is in use
        descriptorImageInfos[i].setImageView(
            bindings[i].imageView == nullptr ?
            bindings[i].texture->getImageView() :
            *bindings[i].imageView); // Image to be bind to set
        descriptorImageInfos[i].setSampler(
            this->resourceManager->getTextureSampler(
                bindings[i].texture->getSamplerIndex()
            ).getVkSampler()
        );                         // The sampler to use for this descriptor set

        // Descriptor write info
        writeDescriptorSets[i].setDstSet(descriptorSet);
        writeDescriptorSets[i].setDstBinding(uint32_t(i));
        writeDescriptorSets[i].setDstArrayElement(uint32_t(0));
        writeDescriptorSets[i].setDescriptorType(this->perDrawInputLayout.descriptorBindingsTypes[i]);
        writeDescriptorSets[i].setDescriptorCount(uint32_t(1)); // (Num pImageInfos)
        writeDescriptorSets[i].setPImageInfo(&descriptorImageInfos[i]);
    }

    // Update descriptor sets
    this->device->getVkDevice().updateDescriptorSets(
        writeDescriptorSets,
        nullptr
    );
}

void ShaderInput::setFrequencyInput(uint32_t descriptorIndex)
{
    this->bindDescriptorSets[(uint32_t)DescriptorFrequency::PER_DRAW_CALL] =
        &this->perDrawDescriptorSets[descriptorIndex];
}