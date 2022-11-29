#include "pch.h"
#include "ParticleSystemHandler.hpp"
#include "../vulkan/Device.hpp"
#include "../vulkan/RenderPass.hpp"
#include "../vulkan/CommandBufferArray.hpp"
#include "../../resource_management/ResourceManager.hpp"

ParticleSystemHandler::ParticleSystemHandler()
	: physicalDevice(nullptr),
	device(nullptr),
	vma(nullptr),
	transferQueue(nullptr),
	transferCommandPool(nullptr),
	resourceManager(nullptr),
	renderPass(nullptr),
	computeCommandPool(nullptr),
	framesInFlight(0)
{ }

void ParticleSystemHandler::init(
	PhysicalDevice& physicalDevice,
	Device& device, 
	VmaAllocator& vma,
	vk::Queue& transferQueue,
	vk::CommandPool& transferCommandPool,
	ResourceManager& resourceManager,
	RenderPass& renderPass,
	vk::CommandPool& computeCommandPool,
	const uint32_t& framesInFlight)
{
	this->physicalDevice = &physicalDevice;
	this->device = &device;
	this->vma = &vma;
	this->transferQueue = &transferQueue;
	this->transferCommandPool = &transferCommandPool;
	this->resourceManager = &resourceManager;
	this->renderPass = &renderPass;
	this->computeCommandPool = &computeCommandPool;
	this->framesInFlight = framesInFlight;

	// Command buffers
	this->computeCommandBuffers.createCommandBuffers(
		device,
		computeCommandPool,
		framesInFlight
	);
}

void ParticleSystemHandler::initForScene(Scene* scene)
{
	this->cleanup();

	// Initial particle infos
	auto particleSystemView =
		scene->getSceneReg().view<ParticleSystem>();
	particleSystemView.each(
		[&](ParticleSystem& particleSystemComp)
		{
			this->initialParticleInfos.resize(MAX_NUM_PARTICLES_PER_SYSTEM);
			for (uint32_t i = 0; i < MAX_NUM_PARTICLES_PER_SYSTEM; ++i)
			{
				ParticleInfo& particle =
					this->initialParticleInfos[i];

				// TODO: remove this
				particle.transformMatrix =
					glm::translate(glm::mat4(1.0f), glm::vec3(i * 2.1f, 0.0f, 0.0f));

				// Size
				particle.startSize = particleSystemComp.startSize;
				particle.endSize = particleSystemComp.endSize;

				// Color
				particle.startColor = glm::vec4(particleSystemComp.startColor, 1.0f);
				particle.endColor = glm::vec4(particleSystemComp.endColor, 1.0f);
			}
		}
	);

	// Shader input
	this->shaderInput.initForGpuOnlyResources(
		*this->transferQueue,
		*this->transferCommandPool
	);
	this->shaderInput.beginForInput(
		*this->physicalDevice,
		*this->device,
		*this->vma,
		*this->resourceManager,
		this->framesInFlight
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
			DescriptorFrequency::PER_FRAME,
			true,
			this->initialParticleInfos.data()
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
		*this->device,
		this->shaderInput,
		*this->renderPass,
		VertexStreams{},
		"particle.vert.spv",
		"particle.frag.spv"
	);

	// Add all textures for possible use as the texture index
	size_t numTextures = this->resourceManager->getNumTextures();
	for (size_t i = 0; i < numTextures; ++i)
	{
		Texture& texture = this->resourceManager->getTexture(i);

		this->shaderInput.addFrequencyInput(
			{ FrequencyInputBindings{ &texture } }
		);
	}

	// Compute pipeline
	this->computePipeline.createComputePipeline(
		*this->device,
		this->shaderInput,
		"particle.comp.spv"
	);

	// No need to keep the initial information
	this->initialParticleInfos.clear();
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
