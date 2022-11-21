#pragma once

#include <vulkan/vulkan.hpp>

class Device;
class Swapchain;
class Texture;

class RenderPass
{
private:
	vk::RenderPass renderPass;

	Device* device;

public:
	void createRenderPassBase(Device& device, PostProcessHandler& postProcessHandler);
	void createRenderPassShadowMap(Device& device, Texture& shadowMapTexture);
	void createRenderPassBloom(Device& device, PostProcessHandler& postProcessHandler);
	void createRenderPassImgui(Device& device, Swapchain& swapchain);

	void cleanup();

	inline const vk::RenderPass& getVkRenderPass() const { return renderPass; }
};