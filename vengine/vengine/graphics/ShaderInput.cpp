#include "ShaderInput.hpp"
#include "vulkan/PhysicalDevice.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/VulkanDbg.hpp"
#include "../ResourceManagement/ResourceManager.hpp"
#include "Texture.hpp"
#include "../dev/Log.hpp"

void ShaderInput::createDescriptorSetLayout()
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    // Layout for uniform buffers
    std::vector<vk::DescriptorSetLayoutBinding> layoutBindings;
    layoutBindings.resize(this->addedUniformBuffers.size());
    for (size_t i = 0; i < layoutBindings.size(); ++i)
    {
        layoutBindings[i].setBinding(uint32_t(i));                                           // Describes which Binding Point in the shaders this layout is being bound to
        layoutBindings[i].setDescriptorType(vk::DescriptorType::eUniformBuffer);    // Type of descriptor (Uniform, Dynamic uniform, image Sampler, etc.)
        layoutBindings[i].setDescriptorCount(uint32_t(1));                                   // Amount of actual descriptors we're binding, where just binding one; our MVP struct
        layoutBindings[i].setStageFlags(vk::ShaderStageFlagBits::eVertex);               // What Shader Stage we want to bind our Descriptor set to
        layoutBindings[i].setPImmutableSamplers(nullptr);          // Used by Textures; whether or not the Sampler should be Immutable
    }
    
    // Create Descriptor Set Layout with given bindings
    vk::DescriptorSetLayoutCreateInfo layoutCreateInfo{};
    layoutCreateInfo.setBindingCount(static_cast<uint32_t>(layoutBindings.size()));  // Number of Binding infos
    layoutCreateInfo.setPBindings(layoutBindings.data());                            // Array containing the binding infos

    // Create Descriptor Set Layout
    this->descriptorSetLayout = this->device->getVkDevice().createDescriptorSetLayout(layoutCreateInfo);

    VulkanDbg::registerVkObjectDbgInfo("DescriptorSetLayout ViewProjection", vk::ObjectType::eDescriptorSetLayout, reinterpret_cast<uint64_t>(vk::DescriptorSetLayout::CType(this->descriptorSetLayout)));

    
    // Layout for textures
    std::vector<vk::DescriptorSetLayoutBinding> samplerLayoutBindings(
        this->samplersTextureIndex.size());
    for (size_t i = 0; i < samplerLayoutBindings.size(); ++i)
    {
        samplerLayoutBindings[i].setBinding(uint32_t(i));
        samplerLayoutBindings[i].setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
        samplerLayoutBindings[i].setDescriptorCount(uint32_t(1));
        samplerLayoutBindings[i].setStageFlags(vk::ShaderStageFlagBits::eFragment);
        samplerLayoutBindings[i].setPImmutableSamplers(nullptr);
    }

    // Create a descriptor set layout with given bindings for texture
    vk::DescriptorSetLayoutCreateInfo textureLayoutCreateInfo;
    textureLayoutCreateInfo.setBindingCount(static_cast<uint32_t>(samplerLayoutBindings.size()));
    textureLayoutCreateInfo.setPBindings(samplerLayoutBindings.data());

    // Create sampler descriptor set layout
    this->samplerDescriptorSetLayout = this->device->getVkDevice().createDescriptorSetLayout(textureLayoutCreateInfo);
    VulkanDbg::registerVkObjectDbgInfo("DescriptorSetLayout SamplerTexture", vk::ObjectType::eDescriptorSetLayout, reinterpret_cast<uint64_t>(vk::DescriptorSetLayout::CType(this->samplerDescriptorSetLayout)));
}

void ShaderInput::createDescriptorPool()
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    // Create descriptor pool
    vk::DescriptorPoolSize descriptorPoolSize{};
    descriptorPoolSize.setType(vk::DescriptorType::eUniformBuffer);                                     // Descriptors in Set will be of Type Uniform Buffer
    descriptorPoolSize.setDescriptorCount(
        this->framesInFlight * this->addedUniformBuffers.size()); 

    std::vector<vk::DescriptorPoolSize> descriptorPoolSizes
    {
        descriptorPoolSize
    };

    // Data to create Descriptor Pool
    vk::DescriptorPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.setMaxSets(this->framesInFlight * this->addedUniformBuffers.size());             // Max Nr Of descriptor Sets that can be created from the pool,
    poolCreateInfo.setPoolSizeCount(static_cast<uint32_t>(descriptorPoolSizes.size()));   // Based on how many pools we have in our descriptorPoolSizes
    poolCreateInfo.setPPoolSizes(descriptorPoolSizes.data());                        // PoolSizes to create the Descriptor Pool with
    
    this->descriptorPool = this->device->getVkDevice().createDescriptorPool(
        poolCreateInfo);
    VulkanDbg::registerVkObjectDbgInfo("DescriptorPool UniformBuffer ", vk::ObjectType::eDescriptorPool, reinterpret_cast<uint64_t>(vk::DescriptorPool::CType(this->descriptorPool)));


    // Texture Sampler Pool
    vk::DescriptorPoolSize samplerPoolSize{};
    samplerPoolSize.setType(vk::DescriptorType::eCombinedImageSampler);       // This descriptor pool will have descriptors for Image and Sampler combined    
    samplerPoolSize.setDescriptorCount(MAX_NUM_TEXTURES);

    vk::DescriptorPoolCreateInfo samplerPoolCreateInfo{};
    samplerPoolCreateInfo.setMaxSets(MAX_NUM_TEXTURES);
    samplerPoolCreateInfo.setPoolSizeCount(uint32_t(1));
    samplerPoolCreateInfo.setPPoolSizes(&samplerPoolSize);

    this->samplerDescriptorPool = this->device->getVkDevice().createDescriptorPool(
        samplerPoolCreateInfo);
    VulkanDbg::registerVkObjectDbgInfo("DescriptorPool ImageSampler ", vk::ObjectType::eDescriptorPool, reinterpret_cast<uint64_t>(vk::DescriptorPool::CType(this->samplerDescriptorPool)));
}

void ShaderInput::allocateDescriptorSets()
{
    // One descriptor set per frame in flight
    this->perFrameDescriptorSets.resize(this->framesInFlight);

    // Copy our DescriptorSetLayout so we have one per Image (one per UniformBuffer)
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts(
        this->perFrameDescriptorSets.size(),
        this->descriptorSetLayout
    );

    // Descriptor Set Allocation Info
    vk::DescriptorSetAllocateInfo setAllocInfo;
    setAllocInfo.setDescriptorPool(this->descriptorPool);                                   // Pool to allocate descriptors (Set?) from   
    setAllocInfo.setDescriptorSetCount(
        static_cast<uint32_t>(this->perFrameDescriptorSets.size()));
    setAllocInfo.setPSetLayouts(descriptorSetLayouts.data());                               // Layouts to use to allocate sets (1:1 relationship)

    // Allocate all descriptor sets
    this->perFrameDescriptorSets = this->device->getVkDevice().allocateDescriptorSets(
        setAllocInfo);
}

void ShaderInput::createDescriptorSets()
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    // Update all of the Descriptor Set buffer binding
    for (size_t i = 0; i < this->perFrameDescriptorSets.size(); i++)
    {
        std::vector<vk::DescriptorBufferInfo> descriptorBufferInfos;
        std::vector<vk::WriteDescriptorSet> writeDescriptorSets;
        descriptorBufferInfos.resize(this->addedUniformBuffers.size());
        writeDescriptorSets.resize(this->addedUniformBuffers.size());

        // Loop through all uniform buffers
        for (size_t j = 0; j < this->addedUniformBuffers.size(); ++j)
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

        // Update the Descriptor Set with new buffer/binding info
        this->device->getVkDevice().updateDescriptorSets(
            writeDescriptorSets,  // Update all Descriptor sets in writeDescriptorSets vector
            nullptr
        );

        VulkanDbg::registerVkObjectDbgInfo("DescriptorSet[" + std::to_string(i) + "]  UniformBuffer", vk::ObjectType::eDescriptorSet, reinterpret_cast<uint64_t>(vk::DescriptorSet::CType(this->perFrameDescriptorSets[i])));
    }
}

int ShaderInput::addPossibleTexture(
    const uint32_t& textureIndex,
    vk::Sampler& textureSampler)
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
    setAllocateInfo.setDescriptorPool(this->samplerDescriptorPool);
    setAllocateInfo.setDescriptorSetCount(uint32_t(1));
    setAllocateInfo.setPSetLayouts(&this->samplerDescriptorSetLayout);

    // Allocate Descriptor Sets
    vk::DescriptorSet descriptorSet =
        this->device->getVkDevice().allocateDescriptorSets(setAllocateInfo)[0];

    // Tedxture Image info
    vk::DescriptorImageInfo imageInfo;
    imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);     // The Image Layout when it is in use
    imageInfo.setImageView(
        this->resourceManager->getTexture(textureIndex).getImageView()); // Image to be bind to set
    imageInfo.setSampler(textureSampler);                         // the Sampler to use for this Descriptor Set

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
    currentFrame(0),
    pushConstantSize(0),
    pushConstantShaderStage(vk::ShaderStageFlagBits::eAll)
{ }

ShaderInput::~ShaderInput()
{}

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

    // Define the Push Constants values
    this->pushConstantRange.setStageFlags(this->pushConstantShaderStage);    // Push Constant should be available in the Vertex Shader!
    this->pushConstantRange.setOffset(uint32_t(0));                             // Offset into the given data that our Push Constant value is (??)
    this->pushConstantRange.setSize(this->pushConstantSize);                 // Size of the Data being passed
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
    this->createDescriptorSets();

    // Descriptors to bind when rendering
    this->bindDescriptorSets.resize(
        (int) DescriptorFrequency::NUM_FREQUENCY_TYPES);

    this->pipelineLayout.createPipelineLayout(
        *this->device,
        this->descriptorSetLayout,
        this->samplerDescriptorSetLayout,
        this->pushConstantRange
    );
}

void ShaderInput::cleanup()
{
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
    this->device->getVkDevice().destroyDescriptorPool(this->descriptorPool);
    this->device->getVkDevice().destroyDescriptorPool(this->samplerDescriptorPool);

    // Descriptor set layouts
    this->device->getVkDevice().destroyDescriptorSetLayout(this->descriptorSetLayout);
    this->device->getVkDevice().destroyDescriptorSetLayout(this->samplerDescriptorSetLayout);

    // Pipeline layout
    this->pipelineLayout.cleanup();
}

void ShaderInput::updateUniformBuffer(
    const UniformBufferID& id,
    void* data,
    const uint32_t& currentFrame)
{
    this->addedUniformBuffers[id].update(data, currentFrame);
}

void ShaderInput::setCurrentFrame(const uint32_t& currentFrame)
{
    this->currentFrame = currentFrame;

    // Bind descriptor set
    this->bindDescriptorSets[(uint32_t) DescriptorFrequency::PER_FRAME] =
        this->perFrameDescriptorSets[currentFrame];
}

void ShaderInput::setTexture(
    const SamplerID& samplerID, 
    const uint32_t& textureIndex)
{
    // Current texture that sampler will use
    this->samplersTextureIndex[samplerID] = textureIndex;

    // Bind descriptor set
    this->bindDescriptorSets[(uint32_t) DescriptorFrequency::PER_DRAW_CALL] = 
        this->perDrawDescriptorSets[textureIndex];
}