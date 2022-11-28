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

void ParticleSystemHandler::update(const uint32_t& currentFrame)
{
	this->shaderInput.setCurrentFrame(currentFrame);
}

void ParticleSystemHandler::cleanup()
{
	this->pipeline.cleanup();
	this->shaderInput.cleanup();
}
