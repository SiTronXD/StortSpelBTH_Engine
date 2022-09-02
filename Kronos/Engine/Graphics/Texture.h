#pragma once

#include <vector>
#include <string>

#include "Buffer.h"

class Renderer;

class Texture
{
private:
	VkImage image;
	VkDeviceMemory imageMemory;
	VkImageView imageView;
	VkSampler sampler;

	Renderer& renderer;

	bool hasStencilComponent(VkFormat format);

	void createImage(
		uint32_t width,
		uint32_t height,
		VkFormat format,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkImage& image,
		VkDeviceMemory& imageMemory);
	void transitionImageLayout(
		VkImage image,
		VkFormat format,
		VkImageLayout oldLayout,
		VkImageLayout newLayout);
	void copyBufferToImage(
		VkBuffer buffer, VkImage image,
		uint32_t width, uint32_t height);

	bool createTextureImage(const std::string& filePath);
	bool createTextureImageView();
	bool createTextureSampler();

public:
	Texture(Renderer& renderer);
	~Texture();

	bool createFromFile(const std::string& filePath);
	bool createAsDepthTexture(uint32_t width, uint32_t height);

	void cleanup();

	// Vulkan
	static VkFormat findSupportedFormat(
		VkPhysicalDevice physicalDevice,
		const std::vector<VkFormat>& candidates,
		VkImageTiling tiling,
		VkFormatFeatureFlags features);
	static VkFormat findDepthFormat(VkPhysicalDevice physicalDevice);
	static VkImageView createImageView(
		VkDevice device,
		VkImage image,
		VkFormat format,
		VkImageAspectFlags aspectFlags
	);

	inline VkImage& getVkImage() { return this->image; }
	inline VkDeviceMemory& getVkImageMemory() { return this->imageMemory; }
	inline VkImageView& getVkImageView() { return this->imageView; }
	inline VkSampler& getVkSampler() { return this->sampler; }
};