#pragma once

#include <map>
#include <typeindex>
#include <typeinfo>
#include "../ShaderInput.hpp"
#include "../MeshData.hpp"

class Device;
class RenderPass;

class Pipeline
{
private:
	vk::Pipeline pipeline{};

	Device* device;

	vk::ShaderModule createShaderModule(
		const std::vector<char>& code);

	template <typename T>
	void insertBindingFromStream(
		const std::vector<T>& dataStream,
		std::vector<vk::VertexInputBindingDescription>& 
			outputBindingDescs);

	template <typename T>
	void insertAttributeFromStream(
		const std::vector<T>& dataStream,
		const vk::Format& format,
		std::vector<vk::VertexInputAttributeDescription>& 
			outputAttributeDescs);

	bool hasBeenCreated;

public:
	Pipeline();
	~Pipeline();

	void createPipeline(
		Device& device,
		ShaderInput& shaderInput,
		RenderPass& renderPass,
		const VertexStreams& targetVertexStream,
		const std::string& vertexShaderName,
		const std::string& fragmentShaderName = "shader.frag.spv",
		const bool& depthTestingEnabled = true,
		const bool& wireframe = false,
		const bool& backfaceCulling = true,
		const vk::PrimitiveTopology& topology = 
			vk::PrimitiveTopology::eTriangleList);

	void cleanup();

	inline const vk::Pipeline& getVkPipeline() const
	{ return this->pipeline; }
};