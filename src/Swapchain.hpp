#pragma once

#include "Utilities.hpp"
#include "TempPCH.hpp"

class Window;
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

	Window* window;
	Device* device;
	vk::SurfaceKHR* surface;
	QueueFamilyIndices* queueFamilies;

	vk::SurfaceFormat2KHR chooseBestSurfaceFormat(
		const std::vector<vk::SurfaceFormat2KHR >& formats);
	vk::PresentModeKHR chooseBestPresentationMode(
		const std::vector<vk::PresentModeKHR>& presentationModes);
	vk::Extent2D chooseBestImageResolution(
		const vk::SurfaceCapabilities2KHR& surfaceCapabilities);

	void recreateCleanup();

public:
	Swapchain();
	~Swapchain();

	void createSwapchain(
		Window& window,
		Device& device,
		vk::SurfaceKHR& surface,
		QueueFamilyIndices& queueFamilies);
	void createFramebuffers(
		vk::RenderPass& renderPass);

	void recreateSwapchain(
		vk::RenderPass& renderPass);

	void cleanup();

	inline uint32_t getWidth() { return this->swapchainExtent.width; }
	inline uint32_t getHeight() { return this->swapchainExtent.height; }
	inline size_t getNumImages() { return this->swapchainImages.size(); }
	inline SwapChainDetails& getDetails() { return this->swapchainDetails; }
	inline SwapChainImage& getImage(const uint32_t& index) { return this->swapchainImages[index]; }
	inline vk::SwapchainKHR& getVkSwapchain() { return this->swapchain; }
	inline vk::Format& getVkFormat() { return this->swapchainImageFormat; }
	inline vk::Extent2D& getVkExtent() { return this->swapchainExtent; }
	inline vk::Framebuffer& getVkFramebuffer(const uint32_t& index) { return this->swapchainFrameBuffers[index]; }
};