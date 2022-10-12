#pragma once

#include "TempPCH.hpp"

class PhysicalDevice;
class Device;

struct ImageCreateData
{
	uint32_t width;
	uint32_t height;
	vk::Format format;
	vk::ImageTiling tiling;
	vk::ImageUsageFlags useFlags;
	VmaAllocation* imageMemory;
};

class Texture
{
private:
    Device* device;
    VmaAllocator* vma;

	vk::ImageView imageView;

	uint32_t textureSamplerIndex;
	uint32_t width;
	uint32_t height;

public:
    VmaAllocation imageMemory;
    vk::Image image;

	Texture(Device& device, VmaAllocator& vma);
	~Texture();

	void init(
		const vk::ImageView& imageView, 
		const uint32_t& width, 
		const uint32_t& height,
		const uint32_t& textureSamplerIndex);

	void cleanup();

	inline const vk::ImageView& getImageView() const { return this->imageView; }
	inline const uint32_t& getWidth() const { return this->width; }
	inline const uint32_t& getHeight() const { return this->height; }
	inline const uint32_t& getSamplerIndex() const { return this->textureSamplerIndex; }

	static vk::Format chooseSupportedFormat(
		PhysicalDevice& physicalDevice,
		const std::vector<vk::Format>& formats,
		const vk::ImageTiling& tiling,
		const vk::FormatFeatureFlagBits& featureFlags
	);

	static vk::Image createImage(
		VmaAllocator& vma,
		ImageCreateData&& imageData,
		const std::string& imageDescription
	);

	static vk::ImageView createImageView(
		Device& device,
		const vk::Image& image, 
		const vk::Format& format, 
		const vk::ImageAspectFlags& aspectFlags
	);

	static void transitionImageLayout(
		Device& device,
		const vk::Queue& queue,
		const vk::CommandPool& commandPool,
		const vk::Image& image,
		const vk::ImageLayout& oldLayout,
		const vk::ImageLayout& newLayout);
};