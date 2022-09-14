#pragma once

#include "TempPCH.hpp"

class PhysicalDevice;
class Device;

class Texture
{
private:
public:
	Texture();
	~Texture();

	static vk::Format chooseSupportedFormat(
		PhysicalDevice& physicalDevice,
		const std::vector<vk::Format>& formats,
		vk::ImageTiling tiling,
		vk::FormatFeatureFlagBits featureFlags
	);

	static vk::Image createImage(
		VmaAllocator& vma,
		createImageData&& imageData, 
		const std::string& imageDescription
	);

	static vk::ImageView createImageView(
		Device& device,
		vk::Image image, 
		vk::Format format, 
		vk::ImageAspectFlags aspectFlags
	);
};