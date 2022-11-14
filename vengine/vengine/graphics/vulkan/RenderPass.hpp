#pragma once

#include <vulkan/vulkan.hpp>

class Device;
class Swapchain;

class RenderPass
{
private:
	vk::RenderPass renderPass;

	Device* device;

public:
	void createRenderPassBase(Device& device, Swapchain& swapchain);
	void createRenderPassImgui(Device& device, Swapchain& swapchain);

	void cleanup();

	inline const vk::RenderPass& getVkRenderPass() const { return renderPass; }
};