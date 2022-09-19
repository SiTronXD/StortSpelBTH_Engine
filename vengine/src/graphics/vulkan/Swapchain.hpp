#pragma once

#include "../Utilities.hpp"
#include "../TempPCH.hpp"

class Window;
class PhysicalDevice;
class Device;
class QueueFamilies;

class Swapchain
{
private:
	vk::SwapchainKHR swapchain;

	SwapchainDetails swapchainDetails{};
	std::vector<vk::Image> swapchainImages;
	std::vector<vk::ImageView> swapchainImageViews;
	std::vector<vk::Framebuffer> swapchainFrameBuffers;

	vk::Format swapchainImageFormat{};
	vk::Extent2D swapchainExtent{};
	uint32_t numMinimumImages;

	std::vector<vk::Image> depthBufferImage;
	std::vector<VmaAllocation> depthBufferImageMemory;
	std::vector<vk::ImageView> depthBufferImageView;
	vk::Format depthFormat{};

	Window* window;
	PhysicalDevice* physicalDevice;
	Device* device;
	vk::SurfaceKHR* surface;
	QueueFamilies* queueFamilies;
	VmaAllocator* vma;

	vk::SurfaceFormat2KHR chooseBestSurfaceFormat(
		const std::vector<vk::SurfaceFormat2KHR >& formats);
	vk::PresentModeKHR chooseBestPresentationMode(
		const std::vector<vk::PresentModeKHR>& presentationModes);
	vk::Extent2D chooseBestImageResolution(
		const vk::SurfaceCapabilities2KHR& surfaceCapabilities);

	void createDepthBuffer();

public:
	Swapchain();
	~Swapchain();

	void createSwapchain(
		Window& window,
		PhysicalDevice& physicalDevice,
		Device& device,
		vk::SurfaceKHR& surface,
		QueueFamilies& queueFamilies,
		VmaAllocator& vma);
	void createFramebuffers(
		vk::RenderPass& renderPass);

	void recreateSwapchain(
		vk::RenderPass& renderPass);

	static void getDetails(
		vk::PhysicalDevice& physDevice,
		vk::SurfaceKHR& surface,
		SwapchainDetails& outputDetails);

	void cleanup(bool destroySwapchain = true);

	inline const uint32_t& getWidth() { return this->swapchainExtent.width; }
	inline const uint32_t& getHeight() { return this->swapchainExtent.height; }
	inline const uint32_t& getNumMinimumImages() { return this->numMinimumImages; }
	inline size_t getNumImages() { return this->swapchainImages.size(); }
	inline size_t getNumDepthBufferImages() { return this->depthBufferImage.size(); }
	inline vk::Image& getImage(const uint32_t& index) { return this->swapchainImages[index]; }
	inline vk::Image& getDepthBufferImage(const uint32_t& index) { return this->depthBufferImage[index]; }
	inline vk::ImageView& getImageView(const uint32_t& index) { return this->swapchainImageViews[index]; }
	inline vk::ImageView& getDepthBufferImageView(const uint32_t& index) { return this->depthBufferImageView[index]; }
	inline vk::SwapchainKHR& getVkSwapchain() { return this->swapchain; }
	inline vk::Format& getVkFormat() { return this->swapchainImageFormat; }
	inline vk::Format& getVkDepthFormat() { return this->depthFormat; }
	inline vk::Extent2D& getVkExtent() { return this->swapchainExtent; }
	inline vk::Framebuffer& getVkFramebuffer(const uint32_t& index) { return this->swapchainFrameBuffers[index]; }
};