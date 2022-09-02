#include "FragmentShader.h"

FragmentShader::FragmentShader(Renderer& renderer)
	: Shader(renderer)
{
}

FragmentShader::FragmentShader(Renderer& renderer, const std::string& filePath)
	: Shader(renderer)
{
	this->createFromFile(filePath);
}

FragmentShader::~FragmentShader()
{
}

void FragmentShader::createFromFile(const std::string& filePath)
{
	// Load shader code and create shader module
	Shader::loadAndCreateShaderModule(filePath);

	// Fragment shader stage create info
	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = Shader::getShaderModule();
	fragShaderStageInfo.pName = "main";
	fragShaderStageInfo.pSpecializationInfo = nullptr; // For shader constants

	Shader::setShaderStage(fragShaderStageInfo);
}
