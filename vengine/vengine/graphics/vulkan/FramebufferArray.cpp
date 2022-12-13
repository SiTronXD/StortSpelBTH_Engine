#include "pch.h"
#include "FramebufferArray.hpp"
#include "RenderPass.hpp"

FramebufferArray::FramebufferArray()
    : device(nullptr)
{ }

void FramebufferArray::create(
    Device& device,
    RenderPass& renderPass,
    const vk::Extent2D& extent,
    const std::vector<std::vector<vk::ImageView>>& attachments,
    const uint32_t& numLayers)
{
    this->create(
        device,
        renderPass,
        std::vector<vk::Extent2D>(attachments.size(), extent),
        attachments,
        numLayers
    );
}

void FramebufferArray::create(
    Device& device,
    RenderPass& renderPass,
    const std::vector<vk::Extent2D>& extents,
    const std::vector<std::vector<vk::ImageView>>& attachments,
    const uint32_t& numLayers)
{
    this->device = &device;

    // Create one framebuffer per attachment
    this->framebuffers.resize(attachments.size());
    for (size_t i = 0; i < this->framebuffers.size(); i++)
    {
        vk::FramebufferCreateInfo framebufferCreateInfo{};
        framebufferCreateInfo.setRenderPass(renderPass.getVkRenderPass());
        framebufferCreateInfo.setAttachmentCount(uint32_t(attachments[i].size()));
        framebufferCreateInfo.setPAttachments(attachments[i].data());
        framebufferCreateInfo.setWidth(extents[i].width);
        framebufferCreateInfo.setHeight(extents[i].height);
        framebufferCreateInfo.setLayers(numLayers);

        this->framebuffers[i] =
            this->device->getVkDevice().createFramebuffer(framebufferCreateInfo);
        VulkanDbg::registerVkObjectDbgInfo("Framebuffer[" + std::to_string(i) + "]", vk::ObjectType::eFramebuffer, reinterpret_cast<uint64_t>(vk::Framebuffer::CType(this->framebuffers[i])));
    }
}

void FramebufferArray::cleanup()
{
    for (auto& framebuffer : this->framebuffers)
    {
        this->device->getVkDevice().destroyFramebuffer(framebuffer);
    }
    this->framebuffers.clear();
}