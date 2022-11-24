#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>

class Device;
class RenderPass;

class FramebufferArray
{
private:
	std::vector<vk::Framebuffer> framebuffers;

	Device* device;

public:
	FramebufferArray();

	// attachments[numFramebuffers][attachmentsPerFramebuffer]
	void create(
		Device& device,
		RenderPass& renderPass,
		const vk::Extent2D& extent,
		const std::vector<std::vector<vk::ImageView>>& attachments);
	void create(
		Device& device,
		RenderPass& renderPass,
		const std::vector<vk::Extent2D>& extents,
		const std::vector<std::vector<vk::ImageView>>& attachments);

	void cleanup();

	inline const vk::Framebuffer& operator[](const uint32_t& index) const
	{ return this->framebuffers[index]; }
};