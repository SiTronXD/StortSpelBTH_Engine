#pragma once

#include "CommandPool.h"
#include "CommandBuffer.h"

class Renderer;

class CommandBufferArray
{
private:
	std::vector<CommandBuffer*> commandBuffers;

	Renderer& renderer;
	CommandPool& commandPool;

public:
	CommandBufferArray(Renderer& renderer, CommandPool& commandPool);
	~CommandBufferArray();

	void createCommandBuffers(size_t numCommandBuffers);

	void cleanup();

	inline CommandBuffer& getCommandBuffer(uint32_t commandBufferIndex) 
		{ return *this->commandBuffers[commandBufferIndex]; }
};