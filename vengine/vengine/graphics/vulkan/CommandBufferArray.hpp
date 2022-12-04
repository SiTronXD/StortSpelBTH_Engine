#pragma once 
 #include "op_overload.hpp"

#include "CommandBuffer.hpp"

class Device;

class CommandBufferArray
{
private:
	std::vector<CommandBuffer> commandBuffers;

public:
	void createCommandBuffers(
		Device& device,
		vk::CommandPool& commandPool,
		const uint32_t& numCommandBuffers);

	inline CommandBuffer& operator[](const uint32_t& index)
	{ return this->commandBuffers[index]; }
	inline size_t getNumCommandBuffers() { return this->commandBuffers.size(); }
};