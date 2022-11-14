#pragma once

#include "../Utilities.hpp"
#include "../TempPCH.hpp"
#include "FramebufferArray.hpp"

class Window;
class PhysicalDevice;
class Device;
class QueueFamilies;
class RenderPass;

// Defines what kind of surface we can create with our given surface
struct SwapchainDetails
{
	vk::SurfaceCapabilities2KHR surfaceCapabilities;       // Surface Properties, Image size/extent
	/*!Describes Information about what the surface can handle:
	 * - minImageCount/maxImageCount         : defines the min/max amount of image our surface can handle
	 * - currentExtent/minExtent/maxExtent   : defines the size of a image
	 * - supportedTransforms/currentTransform: ...
	 * */
	std::vector<vk::SurfaceFormat2KHR> Format;          // Surface Image Formats, RGBA and colorspace (??)
	/*!Describes the format for the Color Space and how the data is represented...
	 * */
	std::vector<vk::PresentModeKHR> presentationMode; // The presentation mode that our Swapchain will use.
	/*!How images should be presented to screen...
	 * */

	[[nodiscard]]
	bool isValid() const {
		return !Format.empty() && !presentationMode.empty();
	}
};

class Swapchain
{
private:
	vk::SwapchainKHR swapchain;

	SwapchainDetails details{};
	std::vector<vk::Image> images;
	std::vector<vk::ImageView> imageViews;
	FramebufferArray framebuffers;

	vk::Format imageFormat{};
	vk::Extent2D extent{};
	uint32_t numMinimumImages;

	vk::Image depthBufferImage;
	VmaAllocation depthBufferImageMemory;
	vk::ImageView depthBufferImageView;
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
		RenderPass& renderPass);

	void recreateSwapchain(
		RenderPass& renderPass);

	static void getDetails(
		vk::PhysicalDevice& physDevice,
		vk::SurfaceKHR& surface,
		SwapchainDetails& outputDetails);

	void cleanup(bool destroySwapchain = true);

	bool canCreateValidSwapchain();

	inline const uint32_t& getWidth() { return this->extent.width; }
	inline const uint32_t& getHeight() { return this->extent.height; }
	inline const uint32_t& getNumMinimumImages() { return this->numMinimumImages; }
	inline size_t getNumImages() { return this->images.size(); }
	inline vk::Image& getImage(const uint32_t& index) { return this->images[index]; }
	inline vk::Image& getDepthBufferImage() { return this->depthBufferImage; }
	inline vk::ImageView& getImageView(const uint32_t& index) { return this->imageViews[index]; }
	inline vk::ImageView& getDepthBufferImageView() { return this->depthBufferImageView; }
	inline vk::SwapchainKHR& getVkSwapchain() { return this->swapchain; }
	inline vk::Format& getVkFormat() { return this->imageFormat; }
	inline vk::Format& getVkDepthFormat() { return this->depthFormat; }
	inline vk::Extent2D& getVkExtent() { return this->extent; }
	inline const vk::Framebuffer& getVkFramebuffer(const uint32_t& index) const { return this->framebuffers[index]; }
};