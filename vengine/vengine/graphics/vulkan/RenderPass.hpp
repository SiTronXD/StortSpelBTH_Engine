#pragma once

#include <vulkan/vulkan.hpp>

class Device;
class Swapchain;
class Texture;
class PostProcessHandler;

class RenderPass
{
private:
	vk::RenderPass renderPass;

	Device* device;

public:
	void createRenderPassBase(
		Device& device, 
		const vk::Format& colorBufferFormat,
		const vk::Format& depthBufferFormat);
	void createRenderPassShadowMap(Device& device, Texture& shadowMapTexture);
	void createRenderPassBloom(Device& device, Texture& hdrRenderTexture);
	void createRenderPassImgui(Device& device, Swapchain& swapchain);

	void cleanup();

	inline const vk::RenderPass& getVkRenderPass() const { return renderPass; }
};