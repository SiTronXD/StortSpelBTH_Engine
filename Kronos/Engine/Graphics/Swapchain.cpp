#include <algorithm>

#include "Swapchain.h"
#include "Renderer.h"

void Swapchain::createImageViews()
{
	// Create an image view for each swapchain image
	this->imageViews.resize(this->images.size());
	for (size_t i = 0; i < this->images.size(); ++i)
	{
		this->imageViews[i] = Texture::createImageView(
			this->renderer.getVkDevice(),
			this->images[i],
			this->imageFormat,
			VK_IMAGE_ASPECT_COLOR_BIT
		);
	}
}

void Swapchain::chooseSwapSurfaceFormat(
	const std::vector<VkSurfaceFormatKHR>& availableFormats,
	VkSurfaceFormatKHR& output)
{
	// Find specific format/color space combination
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
			availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			output = availableFormat;
			return;
		}
	}

	// Use the first format/color space combination
	Log::error("No optimal swapchain format was found. First format was chosen.");
	output = availableFormats[0];
}

void Swapchain::chooseSwapPresentMode(
	const std::vector<VkPresentModeKHR>& availablePresentModes,
	VkPresentModeKHR& output)
{
	// Find specific present mode
	for (const auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			output = availablePresentMode;
			return;
		}
	}

	// Guaranteed to be available
	output = VK_PRESENT_MODE_FIFO_KHR;
	return;
}

void Swapchain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities,
	VkExtent2D& output)
{
	if (capabilities.currentExtent.width != ~uint32_t(0))
	{
		output = capabilities.currentExtent;
		return;
	}
	else
	{
		// Get framebuffer size
		int width, height;
		this->renderer.getWindow().getFramebufferSize(width, height);

		VkExtent2D actualExtent =
		{
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(
			actualExtent.width,
			capabilities.minImageExtent.width,
			capabilities.maxImageExtent.width
		);
		actualExtent.height = std::clamp(
			actualExtent.height,
			capabilities.minImageExtent.height,
			capabilities.maxImageExtent.height
		);

		output = actualExtent;
		return;
	}
}

Swapchain::Swapchain(Renderer& renderer)
	: renderer(renderer),
	depthTexture(renderer),
	swapchain(VK_NULL_HANDLE),
	imageFormat(VK_FORMAT_A1R5G5B5_UNORM_PACK16),
	extent(VkExtent2D{}),
	minImageCount(0)
{
}

Swapchain::~Swapchain()
{
}

void Swapchain::createSwapchain()
{
	// Reusable variables
	VkSurfaceKHR& surface = this->renderer.getSurface().getVkSurface();
	VkDevice& device = this->renderer.getVkDevice();

	// Swap chain support
	SwapchainSupportDetails swapchainSupport{};
	Swapchain::querySwapChainSupport(surface, this->renderer.getVkPhysicalDevice(), swapchainSupport);

	// Format, present mode and extent
	VkSurfaceFormatKHR surfaceFormat{};
	this->chooseSwapSurfaceFormat(swapchainSupport.formats, surfaceFormat);
	VkPresentModeKHR presentMode{};
	this->chooseSwapPresentMode(swapchainSupport.presentModes, presentMode);
	VkExtent2D extent{};
	this->chooseSwapExtent(swapchainSupport.capabilities, extent);

	// Image count
	this->minImageCount = swapchainSupport.capabilities.minImageCount;
	uint32_t imageCount = minImageCount + 1;
	if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount)
	{
		imageCount = swapchainSupport.capabilities.maxImageCount;
	}

	// Swap chain create info
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	// How swap chain images are used across multiple queue families
	QueueFamilyIndices& indices = this->renderer.getQueueFamilies().getIndices();
	uint32_t queueFamilyIndices[] =
	{
		indices.graphicsFamily.value(),
		indices.presentFamily.value()
	};
	if (indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;	// Clip pixels overlapped by other windows
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	// Create swapchain
	if (vkCreateSwapchainKHR(
		device, 
		&createInfo, 
		nullptr, 
		&this->swapchain) != VK_SUCCESS)
	{
		Log::error("Failed to created swapchain.");
		return;
	}

	// We've only specified the minimum number of images, so the implementation
	// could create more.
	vkGetSwapchainImagesKHR(device, this->swapchain, &imageCount, nullptr);
	this->images.resize(imageCount);
	vkGetSwapchainImagesKHR(device, this->swapchain, &imageCount, this->images.data());

	// Save format and extent
	this->imageFormat = surfaceFormat.format;
	this->extent = extent;

	// Create image views
	this->createImageViews();
}

void Swapchain::createFramebuffers()
{
	// Create depth texture that the frame buffers will use
	this->depthTexture.createAsDepthTexture(
		this->getWidth(),
		this->getHeight()
	);

	// Create one framebuffer for each swapchain image view
	this->framebuffers.resize(this->imageViews.size());
	for (size_t i = 0; i < this->imageViews.size(); ++i)
	{
		std::array<VkImageView, 2> attachments =
		{
			this->imageViews[i],
			depthTexture.getVkImageView()
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = this->renderer.getRenderPass().getVkRenderPass();
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = this->getWidth();
		framebufferInfo.height = this->getHeight();
		framebufferInfo.layers = 1;
		if (vkCreateFramebuffer(
			this->renderer.getVkDevice(),
			&framebufferInfo,
			nullptr,
			&this->framebuffers[i]) != VK_SUCCESS)
		{
			Log::error("Failed to create framebuffer.");
		}
	}
}

void Swapchain::recreate()
{
	Window& window = this->renderer.getWindow();

	// Handle minimization when width/height is 0
	int width = 0, height = 0;
	window.getFramebufferSize(width, height);
	while (width == 0 || height == 0)
	{
		window.getFramebufferSize(width, height);
		window.awaitEvents();
	}

	// Wait for the GPU
	vkDeviceWaitIdle(this->renderer.getVkDevice());

	// Cleanup
	this->cleanup();

	// Recreate
	this->createSwapchain();
	this->createFramebuffers();
}

void Swapchain::cleanup()
{
	VkDevice& device = this->renderer.getVkDevice();

	this->depthTexture.cleanup();

	for (auto framebuffer : this->framebuffers)
		vkDestroyFramebuffer(device, framebuffer, nullptr);

	for (auto imageView : this->imageViews)
		vkDestroyImageView(device, imageView, nullptr);

	vkDestroySwapchainKHR(device, this->swapchain, nullptr);
}

void Swapchain::querySwapChainSupport(
	VkSurfaceKHR surface, 
	VkPhysicalDevice device,
	SwapchainSupportDetails& output)
{
	// Fill in capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		device, surface, &output.capabilities
	);

	// Fill in formats
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(
		device, surface, &formatCount, nullptr
	);
	if (formatCount != 0)
	{
		output.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(
			device, surface, &formatCount, output.formats.data()
		);
	}

	// Fill in presentation modes
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(
		device, surface,
		&presentModeCount, nullptr
	);
	if (presentModeCount != 0)
	{
		output.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(
			device, surface,
			&presentModeCount, output.presentModes.data()
		);
	}
}
