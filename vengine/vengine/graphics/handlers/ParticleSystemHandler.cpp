#include "pch.h"
#include "ParticleSystemHandler.hpp"
#include "../vulkan/Device.hpp"
#include "../vulkan/RenderPass.hpp"
#include "../../resource_management/ResourceManager.hpp"

void ParticleSystemHandler::init(
	PhysicalDevice& physicalDevice,
	Device& device, 
	VmaAllocator& vma,
	ResourceManager& resourceManager,
	RenderPass& renderPass,
	const uint32_t& framesInFlight)
{
	
}

void ParticleSystemHandler::initForScene(
	PhysicalDevice& physicalDevice,
	Device& device,
	VmaAllocator& vma,
	ResourceManager& resourceManager,
	RenderPass& renderPass,
	const uint32_t& framesInFlight)
{
	this->cleanup();

	// Shader input
	this->shaderInput.beginForInput(
		physicalDevice,
		device,
		vma,
		resourceManager,
		framesInFlight
	);
	this->cameraUBO =
		this->shaderInput.addUniformBuffer(
			sizeof(CameraBufferData),
			vk::ShaderStageFlagBits::eVertex,
			DescriptorFrequency::PER_FRAME
		);
	FrequencyInputLayout inputLayout{};
	inputLayout.addBinding(vk::DescriptorType::eCombinedImageSampler);
	this->shaderInput.makeFrequencyInputLayout(inputLayout);
	this->shaderInput.endForInput();
	this->pipeline.createPipeline(
		device,
		this->shaderInput,
		renderPass,
		VertexStreams{},
		"particle.vert.spv",
		"particle.frag.spv"
	);

	// Add all textures for possible use as the texture index
	size_t numTextures = resourceManager.getNumTextures();
	for (size_t i = 0; i < numTextures; ++i)
	{
		Texture& texture = resourceManager.getTexture(i);

		this->shaderInput.addFrequencyInput(
			{ FrequencyInputBindings{ &texture } }
		);
	}
}

void ParticleSystemHandler::update(
	const CameraBufferData& cameraDataUBO, 
	const uint32_t& currentFrame)
{
	this->shaderInput.setCurrentFrame(currentFrame);
	this->shaderInput.updateUniformBuffer(
		this->cameraUBO,
		(void*) &cameraDataUBO
	);
}

void ParticleSystemHandler::cleanup()
{
	this->pipeline.cleanup();
	this->shaderInput.cleanup();
}
