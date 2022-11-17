#pragma once

#include "TempPCH.hpp"

class PhysicalDevice;
class Device;

struct TextureSamplerSettings
{
	// Settings for sampler
	vk::Filter filterMode = vk::Filter::eLinear;
	vk::Bool32 unnormalizedCoordinates = VK_FALSE;
};

struct TextureSettings
{
	TextureSamplerSettings samplerSettings
	{
		vk::Filter::eLinear,
		VK_FALSE
	};

	// Settings for texture
	bool keepCpuPixelInfo = false;
};

struct ImageCreateData
{
	uint32_t width;
	uint32_t height;
	vk::Format format;
	vk::ImageTiling tiling;
	vk::ImageUsageFlags useFlags;
	VmaAllocation* imageMemory;
};

struct Pixel
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
};

class Texture
{
private:
	PhysicalDevice* physicalDevice;
    Device* device;
    VmaAllocator* vma;

	vk::Image image;
	vk::ImageView imageView;
	VmaAllocation imageMemory;
	vk::Format format;

	std::vector<Pixel> pixels;

	uint32_t textureSamplerIndex;
	uint32_t width;
	uint32_t height;

	// Vulkan
	uint32_t descriptorIndex;

public:
	Texture();
	~Texture();

	void createAsDepthTexture(
		PhysicalDevice& physicalDevice,
		Device& device,
		VmaAllocator& vma,
		const uint32_t& width,
		const uint32_t& height,
		const vk::ImageUsageFlagBits& extraUsageFlags = (vk::ImageUsageFlagBits) 0,
		const uint32_t& textureSamplerIndex = 0);
	void create(
		PhysicalDevice& physicalDevice,
		Device& device,
		VmaAllocator& vma,
		vk::Queue& transferQueue,
		vk::CommandPool& transferCommandPool,
		stbi_uc* imageData,
		const uint32_t& width,
		const uint32_t& height,
		const TextureSettings& textureSettings,
		const uint32_t& textureSamplerIndex);

	void setDescriptorIndex(const uint32_t& descriptorIndex);

	void cleanup();

	inline const vk::ImageView& getImageView() const { return this->imageView; }
	inline const vk::Format& getVkFormat() const { return this->format; }
	inline const uint32_t& getWidth() const { return this->width; }
	inline const uint32_t& getHeight() const { return this->height; }
	inline const uint32_t& getSamplerIndex() const { return this->textureSamplerIndex; }
	inline const uint32_t& getDescriptorIndex() const { return this->descriptorIndex; }
	inline const Pixel& getCpuPixel(const uint32_t& x, const uint32_t& y) const { return this->pixels[y * this->width + x]; }

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

	static void settingsToString(
		const TextureSettings& samplerSettings,
		std::string& outputString);
};