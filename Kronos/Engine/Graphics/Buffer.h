#pragma once

#include <vector>
#include <array>

#include <vulkan/vulkan.h>

class Renderer;

class Buffer
{
private:
	VkBuffer buffer;
	VkDeviceMemory bufferMemory;

	std::vector<VkBuffer> bufferVec;
	std::vector<VkDeviceMemory> bufferMemoryVec;

	Renderer& renderer;

protected:
	inline Renderer& getRenderer() { return this->renderer; }

public:
	Buffer(Renderer& renderer);
	virtual ~Buffer();

	static uint32_t findMemoryType(
		VkPhysicalDevice physicalDevice, 
		uint32_t typeFilter,
		VkMemoryPropertyFlags properties);
	static void createBuffer(
		VkPhysicalDevice physicalDevice,
		VkDevice device,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkBuffer& buffer,
		VkDeviceMemory& bufferMemory);
	static void copyBuffer(
		Renderer& renderer,
		VkBuffer srcBuffer,
		VkBuffer dstBuffer,
		VkDeviceSize size);

	void cleanup();

	inline VkBuffer& getVkBuffer() { return this->buffer; }
	inline VkDeviceMemory& getVkBufferMemory() { return this->bufferMemory; }
	inline std::vector<VkBuffer>& getVkBufferVec() { return this->bufferVec; }
	inline std::vector<VkDeviceMemory>& getVkBufferMemoryVec() { return this->bufferMemoryVec; }
};