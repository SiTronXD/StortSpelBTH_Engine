#include "IndexBuffer.h"

#include "Renderer.h"
#include <memory.h>
#include <wchar.h>

void IndexBuffer::createStaticGpuIndexBuffer(const std::vector<uint32_t>& indices)
{
	// Resusable references
	Renderer& renderer = Buffer::getRenderer();
	VkPhysicalDevice& physicalDevice =
		renderer.getVkPhysicalDevice();
	VkDevice& device =
		renderer.getVkDevice();

	this->bufferSize = sizeof(indices[0]) * indices.size();

	// Create staging buffer
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	Buffer::createBuffer(
		physicalDevice,
		device,
		this->bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingBufferMemory
	);

	// Map memory
	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, this->bufferSize, 0, &data);
	memcpy(data, indices.data(), (size_t) this->bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	// Create real buffer
	Buffer::createBuffer(
		physicalDevice,
		device,
		this->bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		Buffer::getVkBuffer(),
		Buffer::getVkBufferMemory()
	);

	// Copy from staging buffer to real buffer
	Buffer::copyBuffer(renderer, stagingBuffer, Buffer::getVkBuffer(), this->bufferSize);

	// Deallocate staging buffer
	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void IndexBuffer::createDynamicCpuIndexBuffer(const std::vector<uint32_t>& indices)
{
	// Resusable references
	Renderer& renderer = Buffer::getRenderer();
	VkPhysicalDevice& physicalDevice =
		renderer.getVkPhysicalDevice();
	VkDevice& device =
		renderer.getVkDevice();

	this->bufferSize = sizeof(indices[0]) * indices.size();

	// Create real buffers
	uint32_t maxFramesInFlight = renderer.getMaxFramesInFlight();
	Buffer::getVkBufferVec().resize(maxFramesInFlight);
	Buffer::getVkBufferMemoryVec().resize(maxFramesInFlight);
	for (uint32_t i = 0; i < renderer.getMaxFramesInFlight(); ++i)
	{
		Buffer::createBuffer(
			physicalDevice,
			device,
			this->bufferSize,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			Buffer::getVkBufferVec()[i],
			Buffer::getVkBufferMemoryVec()[i]
		);

		this->updateIndexBuffer(indices, i);
	}
}

IndexBuffer::IndexBuffer(Renderer& renderer)
	: Buffer(renderer),
	bufferSize(0),
	isBufferDynamic(false)
{
}

IndexBuffer::~IndexBuffer()
{
}

void IndexBuffer::createIndexBuffer(
	const std::vector<uint32_t>& indices, 
	bool cpuWriteable)
{
	this->isBufferDynamic = cpuWriteable;

	if (!cpuWriteable)
		this->createStaticGpuIndexBuffer(indices);
	else
		this->createDynamicCpuIndexBuffer(indices);
}

void IndexBuffer::updateIndexBuffer(const std::vector<uint32_t>& indices, uint32_t bufferIndex)
{
	Renderer& renderer = Buffer::getRenderer();
	VkDevice& device =
		renderer.getVkDevice();

	// Map memory
	void* data;
	vkMapMemory(device, Buffer::getVkBufferMemoryVec()[bufferIndex], 0, this->bufferSize, 0, &data);
	memcpy(data, indices.data(), (size_t) this->bufferSize);
	vkUnmapMemory(device, Buffer::getVkBufferMemoryVec()[bufferIndex]);
}
