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

public:
    vk::ImageView imageView;
    VmaAllocation imageMemory;
    vk::Image image;

	Texture(Device& device, VmaAllocator& vma);
	~Texture();

	void cleanup();

	inline const vk::ImageView& getImageView() const { return this->imageView; }

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