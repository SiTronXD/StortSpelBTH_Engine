#include "pch.h"
#include "ParticleSystemHandler.hpp"
#include "../vulkan/Device.hpp"
#include "../vulkan/RenderPass.hpp"

void ParticleSystemHandler::init(
	PhysicalDevice& physicalDevice,
	Device& device, 
	VmaAllocator& vma,
	ResourceManager& resourceManager,
	RenderPass& renderPass,
	const uint32_t& framesInFlight)
{
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
	this->shaderInput.endForInput();
	this->pipeline.createPipeline(
		device, 
		this->shaderInput, 
		renderPass,
		VertexStreams{},
		"particle.vert.spv",
		"particle.frag.spv"
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
}

void ParticleSystemHandler::cleanup()
{
	this->pipeline.cleanup();
	this->shaderInput.cleanup();
}
