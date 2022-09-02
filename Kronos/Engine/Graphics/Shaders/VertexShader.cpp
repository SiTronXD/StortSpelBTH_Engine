#include "VertexShader.h"

VertexShader::VertexShader(Renderer& renderer)
	: Shader(renderer)
{
}

VertexShader::VertexShader(Renderer& renderer, const std::string& filePath)
	: Shader(renderer)
{
	this->createFromFile(filePath);
}

VertexShader::~VertexShader()
{
}

void VertexShader::createFromFile(const std::string& filePath)
{
	// Load shader code and create shader module
	Shader::loadAndCreateShaderModule(filePath);

	// Vertex shader stage create info
	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = Shader::getShaderModule();
	vertShaderStageInfo.pName = "main";
	vertShaderStageInfo.pSpecializationInfo = nullptr; // For shader constants

	Shader::setShaderStage(vertShaderStageInfo);
}
