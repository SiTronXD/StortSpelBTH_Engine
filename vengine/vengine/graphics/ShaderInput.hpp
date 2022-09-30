#pragma once

#include "vulkan/PipelineLayout.hpp"
#include "vulkan/UniformBuffer.hpp"
#include "vulkan/StorageBuffer.hpp"

class PhysicalDevice;
class Device;
class ResourceManager;
class Texture;

using UniformBufferID = uint32_t;
using StorageBufferID = uint32_t;
using SamplerID = uint32_t;

enum class DescriptorFrequency
{
	PER_FRAME,
	// PER_MESH, // TODO: add functionality for this
	PER_DRAW_CALL,

	NUM_FREQUENCY_TYPES
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
	vk::DescriptorPool perDrawPool{};

	vk::DescriptorSetLayout perFrameSetLayout{};
	vk::DescriptorSetLayout perDrawSetLayout{};
	vk::PushConstantRange pushConstantRange{};

	std::vector<UniformBuffer> addedUniformBuffers;
	std::vector<StorageBuffer> addedStorageBuffers; 

	std::vector<vk::DescriptorSet> perFrameDescriptorSets;
	// std::vector<vk::DescriptorSet> perMeshDescriptorSets; // TODO: add functionality for this
	std::vector<vk::DescriptorSet> perDrawDescriptorSets;

	std::vector<uint32_t> samplersTextureIndex;

	std::vector<vk::DescriptorSetLayout> bindDescriptorSetLayouts;
	std::vector<vk::DescriptorSet> bindDescriptorSets;

	uint32_t pushConstantSize;
	vk::ShaderStageFlagBits pushConstantShaderStage;
	bool usePushConstant;

	void createDescriptorSetLayout();
	void createDescriptorPool();
	void allocateDescriptorSets();
	void createDescriptorSets();

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
	UniformBufferID addUniformBuffer(
		const size_t& contentsSize);
    StorageBufferID addStorageBuffer(
		const size_t& contentsSize);
	SamplerID addSampler();
	void endForInput();

	void cleanup();

	void updateUniformBuffer(
		const UniformBufferID& id,
		void* data,
		const uint32_t& currentFrame);
	void setCurrentFrame(const uint32_t& currentFrame);
	void setTexture(
		const SamplerID& samplerID,
		const uint32_t& textureIndex);

	int addPossibleTexture(
		const uint32_t& textureIndex,
		vk::Sampler& textureSampler);

	inline const vk::ShaderStageFlagBits& getPushConstantShaderStage() const { return this->pushConstantShaderStage; }
	inline const uint32_t& getPushConstantSize() const { return this->pushConstantSize; }
	inline const std::vector<vk::DescriptorSetLayout>& getBindDescriptorSetLayouts() const
		{ return this->bindDescriptorSetLayouts; }
	inline const std::vector<vk::DescriptorSet>& getBindDescriptorSets() const 
		{ return this->bindDescriptorSets; }
	inline const PipelineLayout& getPipelineLayout() const 
		{ return this->pipelineLayout; }
	inline const vk::PushConstantRange& getPushConstantRange() const
		{ return this->pushConstantRange; }
	inline const bool& getIsUsingPushConstant() const 
		{ return this->usePushConstant; }
};