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
	this->particleInfoSBO =
		this->shaderInput.addStorageBuffer(
			sizeof(ParticleInfo) * MAX_NUM_PARTICLES_PER_SYSTEM,
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

	// TODO: remove this
	this->particleInfos.resize(MAX_NUM_PARTICLES_PER_SYSTEM);
	for (uint32_t i = 0; i < 64; ++i)
	{
		this->particleInfos[i].transformMatrix =
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f / (i*0.5f+1.0f))) *
			glm::translate(glm::mat4(1.0f), glm::vec3(i * 1.1f, 0.0f, 0.0f));
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

	this->shaderInput.updateStorageBuffer(
		this->particleInfoSBO,
		this->particleInfos.data()
	);
}

void ParticleSystemHandler::cleanup()
{
	this->pipeline.cleanup();
	this->shaderInput.cleanup();
}
