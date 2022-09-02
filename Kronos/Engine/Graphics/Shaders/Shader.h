#pragma once

#include <string>
#include <vector>

#include <vulkan/vulkan.h>

class Renderer;

class Shader
{
private:
	VkShaderModule shaderModule;
	VkPipelineShaderStageCreateInfo shaderStage;

	Renderer& renderer;

protected:
	void readFile(const std::string& filePath, std::vector<char>& output);
	void loadAndCreateShaderModule(const std::string& filePath);
	void setShaderStage(const VkPipelineShaderStageCreateInfo& shaderStage);

	VkShaderModule createShaderModule(const std::vector<char>& code);

	inline const VkShaderModule& getShaderModule() { return this->shaderModule; }

public:
	Shader(Renderer& renderer);
	virtual ~Shader();

	void cleanup();

	inline const VkPipelineShaderStageCreateInfo& getShaderStage() { return this->shaderStage; }
};