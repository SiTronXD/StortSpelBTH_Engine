#pragma once

#include "Buffer.hpp"

class Device;

class VertexBufferArray
{
private:
	// pos_0, uv_0, pos_1, uv_1, ... , pos_FiF-1, uv_FiF-1
	std::vector<vk::Buffer> vertexBuffers;
	std::vector<VmaAllocation> vertexBufferMemories;

	Device* device;
	VmaAllocator* vma;
	vk::Queue* transferQueue;
	vk::CommandPool* transferCommandPool;

public:
	VertexBufferArray();
	VertexBufferArray(VertexBufferArray&& ref);

	void create(
		Device& device,
		VmaAllocator& vma,
		vk::Queue& transferQueue,
		vk::CommandPool& transferCommandPool);

	template <typename T>
	void addVertexBuffer(
		const std::vector<T>& dataStream);

	void cleanup();

	inline const size_t getNumVertexBuffers() const { return this->vertexBuffers.size(); }
	inline const std::vector<vk::Buffer>& getVertexBuffers() const
	{ return this->vertexBuffers; }
};