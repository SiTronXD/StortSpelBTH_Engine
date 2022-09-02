#include "Buffer.h"

#include "Renderer.h"

Buffer::Buffer(Renderer& renderer)
	: renderer(renderer),
	buffer(VK_NULL_HANDLE),
	bufferMemory(VK_NULL_HANDLE)
{
}

Buffer::~Buffer()
{
}

uint32_t Buffer::findMemoryType(
	VkPhysicalDevice physicalDevice,
	uint32_t typeFilter, 
	VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
	{
		if (typeFilter & (1 << i) &&
			(memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	Log::error("Failed to find suitable memory type.");
	return uint32_t(~0);
}

void Buffer::createBuffer(
	VkPhysicalDevice physicalDevice,
	VkDevice device,
	VkDeviceSize size, 
	VkBufferUsageFlags usage, 
	VkMemoryPropertyFlags properties, 
	VkBuffer& buffer, 
	VkDeviceMemory& bufferMemory)
{
	// Create buffer
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	if (vkCreateBuffer(
		device,
		&bufferInfo,
		nullptr,
		&buffer) != VK_SUCCESS)
	{
		Log::error("Failed to create buffer.");
	}

	// Get memory requirements from buffer
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(
		device,
		buffer,
		&memRequirements
	);

	// Allocate memory for buffer
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = Buffer::findMemoryType(
		physicalDevice,
		memRequirements.memoryTypeBits,
		properties
	);
	if (vkAllocateMemory(
		device,
		&allocInfo,
		nullptr,
		&bufferMemory) != VK_SUCCESS)
	{
		Log::error("Failed to allocate vertex buffer memory.");
	}

	// Bind memory to buffer
	vkBindBufferMemory(
		device,
		buffer,
		bufferMemory,
		0
	);
}

void Buffer::copyBuffer(
	Renderer& renderer,
	VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = CommandBuffer::beginSingleTimeCommands(renderer);

	// Record copy buffer
	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	CommandBuffer::endSingleTimeCommands(
		renderer,
		commandBuffer
	);
}

void Buffer::cleanup()
{
	vkDestroyBuffer(Buffer::getRenderer().getVkDevice(), this->buffer, nullptr);
	vkFreeMemory(Buffer::getRenderer().getVkDevice(), this->bufferMemory, nullptr);

	for (size_t i = 0; i < this->bufferVec.size(); ++i)
	{
		vkDestroyBuffer(Buffer::getRenderer().getVkDevice(), this->bufferVec[i], nullptr);
		vkFreeMemory(Buffer::getRenderer().getVkDevice(), this->bufferMemoryVec[i], nullptr);
	}
	this->bufferVec.clear();
	this->bufferMemoryVec.clear();
}
