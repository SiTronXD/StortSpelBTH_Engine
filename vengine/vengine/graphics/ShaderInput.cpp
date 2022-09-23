#include "ShaderInput.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/VulkanDbg.hpp"

void ShaderInput::createDescriptorSetLayout()
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    // - CREATE UNIFORM VALUES DESCRIPTOR SET LAYOUT -

    // UboViewProjection binding Info
    vk::DescriptorSetLayoutBinding vpLayoutBinding;
    vpLayoutBinding.setBinding(uint32_t(0));                                           // Describes which Binding Point in the shaders this layout is being bound to
    vpLayoutBinding.setDescriptorType(vk::DescriptorType::eUniformBuffer);    // Type of descriptor (Uniform, Dynamic uniform, image Sampler, etc.)
    vpLayoutBinding.setDescriptorCount(uint32_t(1));                                   // Amount of actual descriptors we're binding, where just binding one; our MVP struct
    vpLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eVertex);               // What Shader Stage we want to bind our Descriptor set to
    vpLayoutBinding.setPImmutableSamplers(nullptr);//vknullhandle??          // Used by Textures; whether or not the Sampler should be Immutable

    // Adding the Bindings to a Vector in order to submit all the DescriptorSetLayout Bindings! 
    std::vector<vk::DescriptorSetLayoutBinding> layoutBindings{
        vpLayoutBinding
        // Left for Reference; we dont use Dynamic Uniform Buffers for our Model Matrix anymore.
        //,modelLayoutBinding
    };

    // Create Descriptor Set Layout with given bindings
    vk::DescriptorSetLayoutCreateInfo layoutCreateInfo{};
    layoutCreateInfo.setBindingCount(static_cast<uint32_t>(layoutBindings.size()));  // Number of Binding infos
    layoutCreateInfo.setPBindings(layoutBindings.data());                            // Array containing the binding infos

    // Create Descriptor Set Layout
    this->descriptorSetLayout = this->device->getVkDevice().createDescriptorSetLayout(layoutCreateInfo);

    VulkanDbg::registerVkObjectDbgInfo("DescriptorSetLayout ViewProjection", vk::ObjectType::eDescriptorSetLayout, reinterpret_cast<uint64_t>(vk::DescriptorSetLayout::CType(this->descriptorSetLayout)));

    // - CREATE TEXTURE SAMPLER DESCRIPTOR SET LAYOUT -
    // Texture Binding Info
    vk::DescriptorSetLayoutBinding samplerLayoutBinding;
    samplerLayoutBinding.setBinding(uint32_t(0));                                   // This can be 0 too, as it will be for a different Descriptor Set, Descriptor set 1 (previous was Descriptor Set 0)! 
    samplerLayoutBinding.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
    samplerLayoutBinding.setDescriptorCount(uint32_t(1));
    samplerLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eFragment);     // The Stage the descriptor layout will pass to will be the Fragment Shader
    samplerLayoutBinding.setPImmutableSamplers(nullptr);

    // Create a Descriptor Set Layout with given bindings for texture
    vk::DescriptorSetLayoutCreateInfo textureLayoutCreateInfo;
    textureLayoutCreateInfo.setBindingCount(uint32_t(1));
    textureLayoutCreateInfo.setPBindings(&samplerLayoutBinding);

    // create Descriptor Set Layout
    this->samplerDescriptorSetLayout = this->device->getVkDevice().createDescriptorSetLayout(textureLayoutCreateInfo);
    VulkanDbg::registerVkObjectDbgInfo("DescriptorSetLayout SamplerTexture", vk::ObjectType::eDescriptorSetLayout, reinterpret_cast<uint64_t>(vk::DescriptorSetLayout::CType(this->samplerDescriptorSetLayout)));
}

void ShaderInput::createDescriptorPool()
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    // - CRTEATE UNIFORM DESCRIPTOR POOL -

    // Pool Size is definied by the Type of the Descriptors times number of those Descriptors
    // viewProjection uniform Buffer Pool size
    vk::DescriptorPoolSize viewProjection_poolSize{};
    viewProjection_poolSize.setType(vk::DescriptorType::eUniformBuffer);                                     // Descriptors in Set will be of Type Uniform Buffer    
    viewProjection_poolSize.setDescriptorCount(this->framesInFlight); // How many Descriptors we want, we want One uniformBuffer so we its only the size of our uniformBuffer

    std::vector<vk::DescriptorPoolSize> descriptorPoolSizes
    {
        viewProjection_poolSize
    };

    // Data to create Descriptor Pool
    vk::DescriptorPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.setMaxSets(this->framesInFlight);             // Max Nr Of descriptor Sets that can be created from the pool, 
    // Same as the number of buffers / images we have. 
    poolCreateInfo.setPoolSizeCount(static_cast<uint32_t>(descriptorPoolSizes.size()));   // Based on how many pools we have in our descriptorPoolSizes
    poolCreateInfo.setPPoolSizes(descriptorPoolSizes.data());                          // PoolSizes to create the Descriptor Pool with

    this->descriptorPool = this->device->getVkDevice().createDescriptorPool(
        poolCreateInfo);
    VulkanDbg::registerVkObjectDbgInfo("DescriptorPool UniformBuffer ", vk::ObjectType::eDescriptorPool, reinterpret_cast<uint64_t>(vk::DescriptorPool::CType(this->descriptorPool)));

    // - CRTEATE SAMPLER DESCRIPTOR POOL -
    // Texture Sampler Pool
    vk::DescriptorPoolSize samplerPoolSize{};
    samplerPoolSize.setType(vk::DescriptorType::eCombinedImageSampler);       // This descriptor pool will have descriptors for Image and Sampler combined    
    // NOTE; Should be treated as seperate Concepts! but this will be enough...
    samplerPoolSize.setDescriptorCount(MAX_OBJECTS);                          // There will be as many Descriptor Sets as there are Objects...
    //NOTE; This WILL limit us to only have ONE texture per Object...

    vk::DescriptorPoolCreateInfo samplerPoolCreateInfo{};
    samplerPoolCreateInfo.setMaxSets(MAX_OBJECTS);
    samplerPoolCreateInfo.setPoolSizeCount(uint32_t(1));
    samplerPoolCreateInfo.setPPoolSizes(&samplerPoolSize);
    /*// NOTE; While the above code does work (The limit of SamplerDescriptorSets are alot higher than Descriptor Sets for Uniform Buffers)
                It's not the best solution.
                The Correct way of doing this would be to take advantage of Array Layers and Texture Atlases.
                Right now we are taking up alot of unncessary memory by enabling us to create unncessary Descriptor Sets,
                We would LIKE to limit the maxSets value to be as low as possible...
    */

    this->samplerDescriptorPool = this->device->getVkDevice().createDescriptorPool(
        samplerPoolCreateInfo);
    VulkanDbg::registerVkObjectDbgInfo("DescriptorPool ImageSampler ", vk::ObjectType::eDescriptorPool, reinterpret_cast<uint64_t>(vk::DescriptorPool::CType(this->samplerDescriptorPool)));
}

void ShaderInput::allocateDescriptorSets()
{
    // Resize Descriptor Set; one Descriptor Set per UniformBuffer
    this->descriptorSets.resize(this->viewProjectionUB->getNumBuffers()); // Since we have a uniform buffer per images, better use size of swapchainImages!

    // Copy our DescriptorSetLayout so we have one per Image (one per UniformBuffer)
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts(
        this->descriptorSets.size(),
        this->descriptorSetLayout
    );

    // Descriptor Set Allocation Info
    vk::DescriptorSetAllocateInfo setAllocInfo;
    setAllocInfo.setDescriptorPool(this->descriptorPool);                                   // Pool to allocate descriptors (Set?) from   
    setAllocInfo.setDescriptorSetCount(static_cast<uint32_t>(this->descriptorSets.size()));
    setAllocInfo.setPSetLayouts(descriptorSetLayouts.data());                               // Layouts to use to allocate sets (1:1 relationship)

    // Allocate all descriptor sets
    this->descriptorSets = this->device->getVkDevice().allocateDescriptorSets(
        setAllocInfo);
}

void ShaderInput::createDescriptorSets()
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    // Update all of the Descriptor Set buffer binding
    for (size_t i = 0; i < this->descriptorSets.size(); i++)
    {
        // - VIEW PROJECTION DESCRIPTOR - 
        // Describe the Buffer info and Data offset Info
        vk::DescriptorBufferInfo viewProjectionBufferInfo;
        viewProjectionBufferInfo.setBuffer(this->viewProjectionUB->getBuffer(i)); // Buffer to get the Data from
        viewProjectionBufferInfo.setOffset(0);
        viewProjectionBufferInfo.setRange((vk::DeviceSize)this->viewProjectionUB->getBufferSize());

        // Data to describe the connection between Binding and Uniform Buffer
        vk::WriteDescriptorSet viewProjectionWriteSet;
        viewProjectionWriteSet.setDstSet(this->descriptorSets[i]);              // Descriptor Set to update
        viewProjectionWriteSet.setDstBinding(uint32_t(0));                                    // Binding to update (Matches with Binding on Layout/Shader)
        viewProjectionWriteSet.setDstArrayElement(uint32_t(0));                                // Index in array we want to update (if we use an array, we do not. thus 0)
        viewProjectionWriteSet.setDescriptorType(vk::DescriptorType::eUniformBuffer);// Type of Descriptor
        viewProjectionWriteSet.setDescriptorCount(uint32_t(1));                                // Amount of Descriptors to update
        viewProjectionWriteSet.setPBufferInfo(&viewProjectionBufferInfo);

        // List of descriptorSetWrites
        std::vector<vk::WriteDescriptorSet> writeDescriptorSets
        {
            viewProjectionWriteSet
        };

        // Update the Descriptor Set with new buffer/binding info
        this->device->getVkDevice().updateDescriptorSets(
            writeDescriptorSets,  // Update all Descriptor sets in writeDescriptorSets vector
            nullptr
        );

        VulkanDbg::registerVkObjectDbgInfo("DescriptorSet[" + std::to_string(i) + "]  UniformBuffer", vk::ObjectType::eDescriptorSet, reinterpret_cast<uint64_t>(vk::DescriptorSet::CType(this->descriptorSets[i])));
    }
}

int ShaderInput::createSamplerDescriptor(
    vk::ImageView textureImage,
    vk::Sampler& textureSampler)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

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
    imageInfo.setImageView(textureImage);                                 // Image to be bind to set
    imageInfo.setSampler(textureSampler);                         // the Sampler to use for this Descriptor Set

    // Descriptor Write Info
    vk::WriteDescriptorSet descriptorWrite;
    descriptorWrite.setDstSet(descriptorSet);
    descriptorWrite.setDstArrayElement(uint32_t(0));
    descriptorWrite.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
    descriptorWrite.setDescriptorCount(uint32_t(1));
    descriptorWrite.setPImageInfo(&imageInfo);

    // Update the new Descriptor Set
    this->device->getVkDevice().updateDescriptorSets(
        uint32_t(1),
        &descriptorWrite,
        uint32_t(0),
        nullptr
    );

    // Add descriptor Set to our list of descriptor Sets
    this->samplerDescriptorSets.push_back(descriptorSet);

    //Return the last created Descriptor set
    return static_cast<int>(this->samplerDescriptorSets.size() - 1);
}

ShaderInput::ShaderInput()
    : device(nullptr),
    vma(nullptr),
    framesInFlight(0),

    viewProjectionUB(nullptr)
{ }

ShaderInput::~ShaderInput()
{}

void ShaderInput::beginForInput(
    Device& device, 
    VmaAllocator& vma,
    const uint32_t& framesInFlight)
{
    this->device = &device;
    this->vma = &vma;
    this->framesInFlight = framesInFlight;
}

void ShaderInput::addUniformBuffer(UniformBuffer& uniformBuffer)
{
    this->viewProjectionUB = &uniformBuffer;
}

void ShaderInput::addPushConstant(const uint32_t& modelMatrixSize)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    // Define the Push Constants values
    this->pushConstantRange.setStageFlags(vk::ShaderStageFlagBits::eVertex);    // Push Constant should be available in the Vertex Shader!
    this->pushConstantRange.setOffset(uint32_t(0));                             // Offset into the given data that our Push Constant value is (??)
    this->pushConstantRange.setSize(modelMatrixSize);                 // Size of the Data being passed
}

void ShaderInput::addSampler()
{
    
}

void ShaderInput::endForInput()
{
    this->createDescriptorSetLayout();
    this->createDescriptorPool();
    this->allocateDescriptorSets();
    this->createDescriptorSets();

    this->pipelineLayout.createPipelineLayout(
        *this->device,
        this->descriptorSetLayout,
        this->samplerDescriptorSetLayout,
        this->pushConstantRange
    );
}

void ShaderInput::cleanup()
{
    this->device->getVkDevice().destroyDescriptorPool(this->descriptorPool);
    this->device->getVkDevice().destroyDescriptorPool(this->samplerDescriptorPool);

    this->device->getVkDevice().destroyDescriptorSetLayout(this->descriptorSetLayout);
    this->device->getVkDevice().destroyDescriptorSetLayout(this->samplerDescriptorSetLayout);

    this->pipelineLayout.cleanup();
}