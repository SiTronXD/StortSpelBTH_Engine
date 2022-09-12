#pragma once

#include "Utilities.hpp"
#include "TempPCH.hpp"

class Device;

class Swapchain
{
private:
	vk::SwapchainKHR swapchain;

	SwapChainDetails swapchainDetails{};
	std::vector<SwapChainImage> swapchainImages;         // We store ALL our SwapChain images here... to have access to them
	std::vector<vk::Framebuffer> swapchainFrameBuffers;

	vk::Format swapchainImageFormat{};
	vk::Extent2D swapchainExtent{};

	Device* device;

	vk::SurfaceFormat2KHR chooseBestSurfaceFormat(
		const std::vector<vk::SurfaceFormat2KHR >& formats);
	vk::PresentModeKHR chooseBestPresentationMode(
		const std::vector<vk::PresentModeKHR>& presentationModes);
	void recreateCleanup();

public:
	Swapchain();
	~Swapchain();

	void createSwapchain(Device& device);
	void createFramebuffers();

	void recreateSwapchain();

	void cleanup();

	inline size_t getNumImages() { return this->swapchainImages.size(); }
	inline vk::SwapchainKHR& getVkSwapchain() { return this->swapchain; }
	inline vk::Format& getVkFormat() { return this->swapchainImageFormat; }
	inline vk::Extent2D& getVkExtent() { return this->swapchainExtent; }
	inline uint32_t getWidth() { return this->swapchainExtent.width; }
	inline uint32_t getHeight() { return this->swapchainExtent.height; }
};