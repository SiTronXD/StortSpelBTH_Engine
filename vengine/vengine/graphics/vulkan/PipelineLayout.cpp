#include "PipelineLayout.hpp"

#include "Device.hpp"
#include "VulkanDbg.hpp"

PipelineLayout::PipelineLayout()
    : device(nullptr)
{
}

PipelineLayout::~PipelineLayout()
{
}

void PipelineLayout::createPipelineLayout(
    Device& device,
    vk::DescriptorSetLayout& descriptorSetLayout,
    vk::DescriptorSetLayout& samplerDescriptorSetLayout,
    vk::PushConstantRange& pushConstantRange)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    this->device = &device;

    // We have two Descriptor Set Layouts, 
    // One for View and Projection matrices Uniform Buffer, and the other one for the texture sampler!
    std::array<vk::DescriptorSetLayout, 2> descriptorSetLayouts
    {
        descriptorSetLayout,
        samplerDescriptorSetLayout
    };

    vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.setSetLayoutCount(
        static_cast<uint32_t>(descriptorSetLayouts.size()));
    pipelineLayoutCreateInfo.setPSetLayouts(
        descriptorSetLayouts.data());
    pipelineLayoutCreateInfo.setPushConstantRangeCount(
        uint32_t(1));                    
    pipelineLayoutCreateInfo.setPPushConstantRanges(
        &pushConstantRange);

    this->pipelineLayout = this->device->getVkDevice().createPipelineLayout(pipelineLayoutCreateInfo);
    VulkanDbg::registerVkObjectDbgInfo("VkPipelineLayout GraphicsPipelineLayout", vk::ObjectType::ePipelineLayout, reinterpret_cast<uint64_t>(vk::PipelineLayout::CType(this->pipelineLayout)));
}

void PipelineLayout::cleanup()
{
    this->device->getVkDevice().destroyPipelineLayout(
        this->pipelineLayout
    );
}
