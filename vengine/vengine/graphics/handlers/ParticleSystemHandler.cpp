#include "pch.h"
#include "ParticleSystemHandler.hpp"
#include "../vulkan/Device.hpp"
#include "../vulkan/RenderPass.hpp"
#include "../vulkan/CommandBufferArray.hpp"
#include "../../resource_management/ResourceManager.hpp"

void ParticleSystemHandler::init(
	PhysicalDevice& physicalDevice,
	Device& device, 
	VmaAllocator& vma,
	ResourceManager& resourceManager,
	RenderPass& renderPass,
	vk::CommandPool& computeCommandPool,
	const uint32_t& framesInFlight)
{
	/*this->computeShaderInput.beginForInput(
		physicalDevice, 
		device, 
		vma,
		resourceManager,
		framesInFlight
	);
	this->particleInfoWriteSBO = 
		this->computeShaderInput.addStorageBuffer(
			sizeof(ParticleInfo) * MAX_NUM_PARTICLES_PER_SYSTEM,
			vk::ShaderStageFlagBits::eCompute,
			DescriptorFrequency::PER_FRAME
		);
	this->computeShaderInput.endForInput();
	this->computePipeline.createComputePipeline(
		device, 
		this->computeShaderInput, 
		"particle.comp.spv"
	);*/

	this->computeCommandBuffers.createCommandBuffers(
		device,
		computeCommandPool,
		framesInFlight
	);
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
			(vk::ShaderStageFlagBits)(uint32_t(vk::ShaderStageFlagBits::eVertex) | uint32_t(vk::ShaderStageFlagBits::eCompute)),
			DescriptorFrequency::PER_FRAME
		);
	this->particleInfoSBO =
		this->shaderInput.addStorageBuffer(
			sizeof(ParticleInfo) * MAX_NUM_PARTICLES_PER_SYSTEM,
			(vk::ShaderStageFlagBits)(uint32_t(vk::ShaderStageFlagBits::eVertex) | uint32_t(vk::ShaderStageFlagBits::eCompute)),
			DescriptorFrequency::PER_FRAME
		);
	this->globalParticleBufferUBO = 
		this->shaderInput.addUniformBuffer(
			sizeof(GlobalParticleBufferData),
			(vk::ShaderStageFlagBits)(uint32_t(vk::ShaderStageFlagBits::eVertex) | uint32_t(vk::ShaderStageFlagBits::eCompute)),
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
			glm::translate(glm::mat4(1.0f), glm::vec3(i * 2.1f, 0.0f, 0.0f));
	}

	// TODO: definitely remove this...
	for (uint32_t i = 0; i < framesInFlight; ++i)
	{
		this->shaderInput.setCurrentFrame(i);
		this->shaderInput.updateStorageBuffer(
			this->particleInfoSBO,
			this->particleInfos.data()
		);
	}
	this->computePipeline.createComputePipeline(
		device,
		this->shaderInput, //this->computeShaderInput, 
		"particle.comp.spv"
	);
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

	// Global particle data
	this->globalParticleData.deltaTime = Time::getDT();
	this->shaderInput.updateUniformBuffer(
		this->globalParticleBufferUBO,
		(void*) &this->globalParticleData
	);
}

void ParticleSystemHandler::cleanup()
{
	this->pipeline.cleanup();
	this->shaderInput.cleanup();

	this->computePipeline.cleanup();
	//this->computeShaderInput.cleanup();
}
