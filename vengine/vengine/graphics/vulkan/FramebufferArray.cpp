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
    const std::vector<std::vector<vk::ImageView>>& attachments)
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
        framebufferCreateInfo.setWidth(extent.width);
        framebufferCreateInfo.setHeight(extent.height);
        framebufferCreateInfo.setLayers(uint32_t(1));

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