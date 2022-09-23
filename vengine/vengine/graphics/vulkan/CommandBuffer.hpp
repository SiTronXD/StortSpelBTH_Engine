#pragma once

#include "../TempPCH.hpp"

class CommandBuffer
{
private:
public:
	CommandBuffer();
	~CommandBuffer();

    static vk::CommandBuffer beginCommandBuffer(
        vk::Device device,
        vk::CommandPool commandPool);

    static void endAndSubmitCommandBuffer(
        vk::Device device,
        vk::CommandPool commandPool, vk::Queue queue,
        vk::CommandBuffer commandBuffer);

    static void endAndSubmitCommandBufferWithFences(
        vk::Device device, 
        vk::CommandPool commandPool, 
        vk::Queue queue,
        vk::CommandBuffer commandBuffer);
};