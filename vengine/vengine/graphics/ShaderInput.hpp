#pragma once

#include "vulkan/PipelineLayout.hpp"
#include "vulkan/UniformBuffer.hpp"
#include "vulkan/StorageBuffer.hpp"
#include "../dev/Log.hpp"

class PhysicalDevice;
class Device;
class ResourceManager;
class Texture;
class TextureSampler;

using UniformBufferID = uint64_t;
using StorageBufferID = uint64_t;

enum class DescriptorFrequency
{
	PER_FRAME,
	PER_MESH,
	PER_DRAW_CALL,

	NUM_FREQUENCY_TYPES
};

struct ResourceHandle
{
	Buffer* buffer = nullptr;
	vk::DescriptorType descriptorType;
	vk::ShaderStageFlagBits shaderStage;
	DescriptorFrequency descriptorFreq;
	bool cpuWritable = true;
};

// Initial layout
#define MAX_NUM_SET_BINDINGS 4
struct FrequencyInputLayout
{
	vk::DescriptorType descriptorBindingsTypes[MAX_NUM_SET_BINDINGS]{};
	uint32_t numBindings = 0;

	void addBinding(const vk::DescriptorType& addedBindingType)
	{
		// Make sure bindings can be added
		if (numBindings >= MAX_NUM_SET_BINDINGS)
		{
			Log::error("Maximum number of bindings for this frequency has been reached.");
			return;
		}

		// Add descriptor type
		descriptorBindingsTypes[numBindings++] = addedBindingType;
	}
};

// One instance per descriptor set. All instances should
// follow the same frequency input layout.
struct FrequencyInputBindings
{
	Texture* texture = nullptr;
};

class ShaderInput
{
private:
	const uint32_t MAX_NUM_PER_DRAW_DESCRIPTOR_SETS = 256;

	PipelineLayout pipelineLayout;

	PhysicalDevice* physicalDevice;
	Device* device;
    VmaAllocator* vma;
    ResourceManager* resourceManager;

	uint32_t currentFrame;
	uint32_t framesInFlight;

	vk::DescriptorSetLayout perFrameSetLayout{};
	vk::DescriptorSetLayout perMeshSetLayout{};
	vk::DescriptorSetLayout perDrawSetLayout{};
	vk::PushConstantRange pushConstantRange{};

	vk::DescriptorPool perFramePool{};
	vk::DescriptorPool perMeshPool{};
	vk::DescriptorPool perDrawPool{};

	// resources[DescriptorFrequency][resource]
	std::vector<std::vector<ResourceHandle>> resources;

	// per...DescriptorSets[frameInFlight][bufferID]
	std::vector<vk::DescriptorSet> perFrameDescriptorSets;
	std::vector<std::vector<vk::DescriptorSet>> perMeshDescriptorSets;
	std::vector<vk::DescriptorSet> perDrawDescriptorSets;

	// Input layout in per draw descriptor set
	FrequencyInputLayout perDrawInputLayout;

	std::vector<vk::DescriptorSetLayout> bindDescriptorSetLayouts;
	std::vector<vk::DescriptorSet*> bindDescriptorSets;

	uint32_t pushConstantSize;
	vk::ShaderStageFlagBits pushConstantShaderStage;
    bool usePushConstant;

	bool hasBeenCreated;

	uint64_t createResourceID(
		const DescriptorFrequency& descriptorFrequency);
	uint32_t getResourceFrequencyIndex(
		const uint64_t& resourceID);
	uint32_t getResourceIndex(
		const uint64_t& resourceID);

	void createDescriptorSetLayouts();
	void createDescriptorPools();
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
		const size_t& contentsSize,
		const vk::ShaderStageFlagBits& shaderStage,
		const DescriptorFrequency& descriptorFrequency);
    StorageBufferID addStorageBuffer(
		const size_t& contentsSize,
		const vk::ShaderStageFlagBits& shaderStage,
		const DescriptorFrequency& descriptorFrequency);
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

	// Only PER_DRAW_CALL for now, but should be made more general.
	void makeFrequencyInputLayout(
		const FrequencyInputLayout& bindingsLayout);
	uint32_t addFrequencyInput(
		const std::vector<FrequencyInputBindings>& bindings);
	void setFrequencyInput(uint32_t descriptorIndex);

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