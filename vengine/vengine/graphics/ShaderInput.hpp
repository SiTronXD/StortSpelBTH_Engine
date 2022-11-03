#pragma once

#include "vulkan/PipelineLayout.hpp"
#include "vulkan/UniformBuffer.hpp"
#include "vulkan/StorageBuffer.hpp"

class PhysicalDevice;
class Device;
class ResourceManager;
class Texture;
class TextureSampler;

using UniformBufferID = uint32_t;
using StorageBufferID = uint32_t;
using SamplerID = uint32_t;

enum class DescriptorFrequency
{
	PER_FRAME,
	PER_MESH,
	PER_DRAW_CALL,

	NUM_FREQUENCY_TYPES
};

struct StorageBufferHandle
{
	StorageBuffer storageBuffer;
	vk::ShaderStageFlagBits shaderStage;
};

class ShaderInput
{
private:
	const uint32_t MAX_NUM_TEXTURES = 250;

	PipelineLayout pipelineLayout;

	PhysicalDevice* physicalDevice;
	Device* device;
    VmaAllocator* vma;
    ResourceManager* resourceManager;

	uint32_t currentFrame;
	uint32_t framesInFlight;

	vk::DescriptorPool perFramePool{};
	vk::DescriptorPool perMeshPool{};
	vk::DescriptorPool perDrawPool{};

	vk::DescriptorSetLayout perFrameSetLayout{};
	vk::DescriptorSetLayout perMeshSetLayout{};
	vk::DescriptorSetLayout perDrawSetLayout{};
	vk::PushConstantRange pushConstantRange{};

	std::vector<UniformBuffer> addedUniformBuffers;
	std::vector<StorageBufferHandle> addedStorageBuffers;

	// per...DescriptorSets[frameInFlight][bufferID]
	std::vector<vk::DescriptorSet> perFrameDescriptorSets;
	std::vector<std::vector<vk::DescriptorSet>> perMeshDescriptorSets;
	std::vector<vk::DescriptorSet> perDrawDescriptorSets;

	std::vector<uint32_t> samplersTextureIndex;

	std::vector<vk::DescriptorSetLayout> bindDescriptorSetLayouts;
	std::vector<vk::DescriptorSet*> bindDescriptorSets;

	uint32_t pushConstantSize;
	vk::ShaderStageFlagBits pushConstantShaderStage;
    bool usePushConstant;

	bool hasBeenCreated;

	void createDescriptorSetLayout();
	void createDescriptorPool();
	void allocateDescriptorSets();
	void updateDescriptorSets();

public:
	ShaderInput();
	~ShaderInput();

	void beginForInput(
		PhysicalDevice& physicalDevice,
		Device& device, 
		VmaAllocator& vma,
		ResourceManager& resourceManager,
		const uint32_t& framesInFlight);
	void addPushConstant(
		const uint32_t& pushConstantSize,
		const vk::ShaderStageFlagBits& pushConstantShaderStage);
	void setNumShaderStorageBuffers(const uint32_t& numStorageBuffers);
	UniformBufferID addUniformBuffer(
		const size_t& contentsSize);
    StorageBufferID addStorageBuffer(
		const size_t& contentsSize,
		const vk::ShaderStageFlagBits& shaderStage = vk::ShaderStageFlagBits::eVertex);
	SamplerID addSampler();
	void endForInput();

	void cleanup();

	void updateUniformBuffer(
		const UniformBufferID& id,
		void* data);
	void updateStorageBuffer(
		const StorageBufferID& id,
		void* data
	);
	void setCurrentFrame(const uint32_t& currentFrame);
	void setStorageBuffer(const StorageBufferID& storageBufferID);
	void setTexture(
		const SamplerID& samplerID,
		const uint32_t& textureIndex);

	int addPossibleTexture(
		const uint32_t& textureIndex,
		TextureSampler& textureSampler);

	inline const vk::ShaderStageFlagBits& getPushConstantShaderStage() const { return this->pushConstantShaderStage; }
	inline const uint32_t& getPushConstantSize() const { return this->pushConstantSize; }
	inline const std::vector<vk::DescriptorSetLayout>& getBindDescriptorSetLayouts() const
		{ return this->bindDescriptorSetLayouts; }
	inline const std::vector<vk::DescriptorSet*>& getBindDescriptorSets() const 
		{ return this->bindDescriptorSets; }
	inline const PipelineLayout& getPipelineLayout() const 
		{ return this->pipelineLayout; }
	inline const vk::PushConstantRange& getPushConstantRange() const
		{ return this->pushConstantRange; }
	inline const bool& getIsUsingPushConstant() const 
		{ return this->usePushConstant; }
};