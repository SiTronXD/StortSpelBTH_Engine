#include "PipelineLayout.hpp"

#include "Device.hpp"
#include "VulkanDbg.hpp"
#include "../ShaderInput.hpp"

PipelineLayout::PipelineLayout()
    : device(nullptr)
{
}

PipelineLayout::~PipelineLayout()
{
}

void PipelineLayout::createPipelineLayout(
    Device& device,
    ShaderInput& shaderInput)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    this->device = &device;

    vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{};

    // Descriptor set layouts
    pipelineLayoutCreateInfo.setSetLayoutCount(
        uint32_t(shaderInput.getBindDescriptorSetLayouts().size()));
    pipelineLayoutCreateInfo.setPSetLayouts(
        shaderInput.getBindDescriptorSetLayouts().data());

    // Push constant range
    if (shaderInput.getIsUsingPushConstant())
    {
        pipelineLayoutCreateInfo.setPushConstantRangeCount(
            uint32_t(1));
        pipelineLayoutCreateInfo.setPPushConstantRanges(
            &shaderInput.getPushConstantRange());
    }

    this->pipelineLayout = this->device->getVkDevice().createPipelineLayout(pipelineLayoutCreateInfo);
    VulkanDbg::registerVkObjectDbgInfo("VkPipelineLayout GraphicsPipelineLayout", vk::ObjectType::ePipelineLayout, reinterpret_cast<uint64_t>(vk::PipelineLayout::CType(this->pipelineLayout)));
}

void PipelineLayout::cleanup()
{
    this->device->getVkDevice().destroyPipelineLayout(
        this->pipelineLayout
    );
}
