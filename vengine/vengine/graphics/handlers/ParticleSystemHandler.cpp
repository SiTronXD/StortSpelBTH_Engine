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
	framesInFlight(0),
	numParticles(0),
	cameraUBO(~0u),
	globalParticleBufferUBO(~0u),
	particleInfoSBO(~0u),
	particleEmitterInfoSBO(~0u)
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
	this->initialParticleInfos.reserve(1024 * 1024);
	this->particleEmitterInfos.reserve(16);
	uint32_t particleIndex = 0;

	auto particleSystemView =
		scene->getSceneReg().view<ParticleSystem>();
	particleSystemView.each(
		[&](ParticleSystem& particleSystemComp)
		{
			uint32_t particleSystemIndex = uint32_t(this->particleEmitterInfos.size());

			// Set base instance offset
			particleSystemComp.baseInstanceOffset = particleIndex;

			for (size_t i = 0; i < particleSystemComp.numParticles; ++i)
			{
				// Create particle info
				this->initialParticleInfos.push_back(ParticleInfo());
				ParticleInfo& particle =
					this->initialParticleInfos[this->initialParticleInfos.size() - 1];

				// Current life timer
				particle.life.x = particleSystemComp.maxlifeTime;

				// Size
				particle.startSize = particleSystemComp.startSize;
				particle.endSize = particleSystemComp.endSize;

				// Acceleration
				particle.acceleration = glm::vec4(particleSystemComp.acceleration, 0.0f);

				// Indices
				particle.indices.x = particleIndex; // Random state
				particle.indices.y = particleSystemIndex; // Particle system index

				// Set particle system index
				particleSystemComp.particleSystemIndex = particleSystemIndex;

				// Next particle index
				particleIndex++;
			}

			// Add particle system emitter info
			this->particleEmitterInfos.push_back(ParticleEmitterInfo());
		}
	);
	this->numParticles = particleIndex;

	// Add dummy emitter/particle if non are found in the scene
	if (this->particleEmitterInfos.size() <= 0)
	{
		this->initialParticleInfos.push_back(ParticleInfo());
		this->particleEmitterInfos.push_back(ParticleEmitterInfo());
	}

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
			sizeof(ParticleInfo) * this->initialParticleInfos.size(),
			(vk::ShaderStageFlagBits)(uint32_t(vk::ShaderStageFlagBits::eVertex) | uint32_t(vk::ShaderStageFlagBits::eCompute)),
			DescriptorFrequency::PER_FRAME,
			true,
			this->initialParticleInfos.data()
		);
	this->particleEmitterInfoSBO =
		this->shaderInput.addStorageBuffer(
			sizeof(ParticleEmitterInfo) * this->particleEmitterInfos.size(),
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
	this->initialParticleInfos.shrink_to_fit();
}

void ParticleSystemHandler::update(
	Scene* scene,
	const CameraBufferData& cameraDataUBO, 
	const uint32_t& currentFrame)
{
	this->shaderInput.setCurrentFrame(currentFrame);
	this->shaderInput.updateUniformBuffer(
		this->cameraUBO,
		(void*) &cameraDataUBO
	);

	// Particle emitters data
	auto particleSystemView =
		scene->getSceneReg().view<Transform, ParticleSystem>();
	particleSystemView.each(
		[&](Transform& transformComp,
			ParticleSystem& particleSystemComp)
		{
			ParticleEmitterInfo& emitterInfo =
				this->particleEmitterInfos[particleSystemComp.particleSystemIndex];

			transformComp.updateMatrix();
			const glm::mat3& rotMat =
				transformComp.getRotationMatrix();

			// Cone position
			emitterInfo.conePos =
				transformComp.position +
				rotMat *
				particleSystemComp.coneSpawnVolume.localPosition;

			// Cone disk radius
			emitterInfo.coneDiskRadius =
				particleSystemComp.coneSpawnVolume.diskRadius;

			// Cone direction
			emitterInfo.coneDir =
				glm::normalize(
					rotMat * 
					particleSystemComp.coneSpawnVolume.localDirection
				);

			// Cone normal
			glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
			if (glm::abs(glm::dot(worldUp, emitterInfo.coneDir)) >= 0.95f)
				worldUp = glm::vec3(1.0f, 0.0f, 0.0f);
			emitterInfo.coneNormal =
				glm::normalize(
					glm::cross(worldUp, emitterInfo.coneDir)
				);

			// Tan theta
			emitterInfo.tanTheta =
				std::tan(
					glm::radians(
						std::clamp(
							particleSystemComp.coneSpawnVolume.coneAngle * 0.5f,
							0.0f, 
							89.0f
						)
					)
				);

			// Start/end colors
			emitterInfo.startColor = particleSystemComp.startColor;
			emitterInfo.endColor = particleSystemComp.endColor;

			// Max life time
			emitterInfo.settings.z = particleSystemComp.maxlifeTime;

			// Respawning
			emitterInfo.shouldRespawn = particleSystemComp.spawn ? 1u : 0u;
			if (particleSystemComp.respawnSetting == RespawnSetting::EXPLOSION)
			{
				if (particleSystemComp.spawn)
				{
					particleSystemComp.spawn = false;
					emitterInfo.settings.z = 0.0f; // Kill all particles this frame
				}
			}
			else
			{
				particleSystemComp.spawnRate = (particleSystemComp.spawn ? 1.0f : 0.0f);
			}
			emitterInfo.settings.x =
				std::clamp(particleSystemComp.spawnRate, 0.0f, 1.0f) * particleSystemComp.maxlifeTime;

			// Velocity strength
			emitterInfo.settings.y = particleSystemComp.velocityStrength;
		}
	);
	this->shaderInput.updateStorageBuffer(
		this->particleEmitterInfoSBO,
		this->particleEmitterInfos.data()
	);

	// Global particle data
	this->globalParticleData.deltaTime = Time::getDT();
	this->globalParticleData.numParticles = this->getNumParticles();
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

	this->particleEmitterInfos.clear();
}
