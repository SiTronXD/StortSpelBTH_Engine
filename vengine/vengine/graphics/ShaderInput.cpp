#include "ShaderInput.hpp"

ShaderInput::ShaderInput()
    : device(nullptr),
    vma(nullptr)
{ }

ShaderInput::~ShaderInput()
{}

void ShaderInput::beginForInput(Device& device, VmaAllocator& vma)
{
    this->device = &device;
    this->vma = &vma;
}

void ShaderInput::addUniformBuffer()
{

}

void ShaderInput::addPushConstant()
{

}

void ShaderInput::addSampler()
{
    
}

void ShaderInput::endForInput()
{
    /*this->pipelineLayout.createPipelineLayout(
        this->device,
        this->descriptorSetLayout,
        this->samplerDescriptorSetLayout,
        this->pushConstantRange
    );*/
}

void cleanup()
{
    
}