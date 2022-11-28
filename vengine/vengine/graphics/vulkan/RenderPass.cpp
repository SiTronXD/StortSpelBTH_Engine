#include "pch.h"
#include "RenderPass.hpp"
#include "Swapchain.hpp"
#include "../handlers/PostProcessHandler.hpp"

void RenderPass::createRenderPassShadowMap(
    Device& device,
    Texture& shadowMapTexture)
{
    this->device = &device;

    // Depth attachment
    vk::AttachmentDescription2 depthAttachment{};
    depthAttachment.setFormat(shadowMapTexture.getVkFormat());
    depthAttachment.setSamples(vk::SampleCountFlagBits::e1);
    depthAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);
    depthAttachment.setStoreOp(vk::AttachmentStoreOp::eStore);
    depthAttachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
    depthAttachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
    depthAttachment.setInitialLayout(vk::ImageLayout::eUndefined);
    depthAttachment.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

    // Depth attachment reference
    vk::AttachmentReference2 depthAttachmentReference{};
    depthAttachmentReference.setAttachment(uint32_t(0));                          // Match the number/ID of the Attachment to the index of the FrameBuffer!
    depthAttachmentReference.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal); // The Layout the Subpass must be in! 

    // Array of our subpasses
    std::array<vk::SubpassDescription2, 1> subPasses{};
    subPasses[0].setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
    subPasses[0].setPDepthStencilAttachment(&depthAttachmentReference);

    // Create render pass
    vk::RenderPassCreateInfo2 renderPassCreateInfo;
    renderPassCreateInfo.setAttachmentCount(uint32_t(1));
    renderPassCreateInfo.setPAttachments(&depthAttachment);
    renderPassCreateInfo.setSubpassCount(static_cast<uint32_t>(subPasses.size()));
    renderPassCreateInfo.setPSubpasses(subPasses.data());

    // TODO: consider overriding implicit dependencies
    // renderPassCreateInfo.setDependencyCount(static_cast<uint32_t> (subpassDependencies.size()));
    // renderPassCreateInfo.setPDependencies(subpassDependencies.data());

    this->renderPass =
        this->device->getVkDevice().createRenderPass2(renderPassCreateInfo);

    VulkanDbg::registerVkObjectDbgInfo("ShadowMapRenderPass", vk::ObjectType::eRenderPass, reinterpret_cast<uint64_t>(vk::RenderPass::CType(this->renderPass)));
}

void RenderPass::createRenderPassBase(
    Device& device, 
    const vk::Format& colorBufferFormat,
    const vk::Format& depthBufferFormat)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    this->device = &device;

    // Color attachment
    vk::AttachmentDescription2 colorAttachment{};
    colorAttachment.setFormat(colorBufferFormat);
    colorAttachment.setSamples(vk::SampleCountFlagBits::e1);
    colorAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);        // When we start the renderpass, first thing to do is to clear since there is no values in it yet
    colorAttachment.setStoreOp(vk::AttachmentStoreOp::eStore);      // How to store it after the RenderPass
    colorAttachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
    colorAttachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
    colorAttachment.setInitialLayout(vk::ImageLayout::eUndefined);            // We dont care what the image layout is when we start. But we do care about what layout it is when it enter the first SubPass! (not handled here)
    colorAttachment.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal); // Should be the same value as it was after the subpass finishes

    // Depth attachment
    vk::AttachmentDescription2 depthAttachment{};
    depthAttachment.setFormat(depthBufferFormat);
    depthAttachment.setSamples(vk::SampleCountFlagBits::e1);
    depthAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);                         // Clear Buffer Whenever we try to load data into (i.e. clear before use it!)
    depthAttachment.setStoreOp(vk::AttachmentStoreOp::eDontCare);                    // Whenever it's used, we dont care what happens with the data... (we dont present it or anything)
    depthAttachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);               // Even though the Stencil i present, we dont plan to use it. so we dont care    
    depthAttachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);             // Even though the Stencil i present, we dont plan to use it. so we dont care
    depthAttachment.setInitialLayout(vk::ImageLayout::eUndefined);                   // We don't care how the image layout is initially, so let it be undefined
    depthAttachment.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal); // Final layout should be Optimal for Depth Stencil attachment!

    // Color attachment reference
    vk::AttachmentReference2 colorAttachmentReference{};
    colorAttachmentReference.setAttachment(uint32_t(0));                          // Match the number/ID of the Attachment to the index of the FrameBuffer!
    colorAttachmentReference.setLayout(vk::ImageLayout::eColorAttachmentOptimal); // The Layout the Subpass must be in! 

    // Depth attachment reference
    vk::AttachmentReference2 depthAttachmentReference{};
    depthAttachmentReference.setAttachment(uint32_t(1));
    depthAttachmentReference.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal); // The layout the subpass must be in! Should be same as 'final layout'(??)

    // Array of our subpasses
    std::array<vk::SubpassDescription2, 1> subPasses{};
    subPasses[0].setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
    subPasses[0].setColorAttachmentCount(uint32_t(1));
    subPasses[0].setPColorAttachments(&colorAttachmentReference);
    subPasses[0].setPDepthStencilAttachment(&depthAttachmentReference);

    // Override the first implicit subpass
    std::array<vk::SubpassDependency2, 1> subpassDependencies{};
    subpassDependencies[0].setSrcSubpass(VK_SUBPASS_EXTERNAL);
    subpassDependencies[0].setDstSubpass(0);
    subpassDependencies[0].setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests);
    subpassDependencies[0].setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests);
    subpassDependencies[0].setSrcAccessMask(vk::AccessFlagBits::eNone);
    subpassDependencies[0].setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite);
    subpassDependencies[0].setDependencyFlags(vk::DependencyFlagBits::eByRegion);

    // Vector with the attachments
    std::array<vk::AttachmentDescription2, 2> attachments
    {
        colorAttachment,
        depthAttachment
    };

    // Create info for render pass
    vk::RenderPassCreateInfo2 renderPassCreateInfo;
    renderPassCreateInfo.setAttachmentCount(static_cast<uint32_t>(attachments.size()));
    renderPassCreateInfo.setPAttachments(attachments.data());
    renderPassCreateInfo.setSubpassCount(static_cast<uint32_t>(subPasses.size()));
    renderPassCreateInfo.setPSubpasses(subPasses.data());
    renderPassCreateInfo.setDependencyCount(static_cast<uint32_t> (subpassDependencies.size()));
    renderPassCreateInfo.setPDependencies(subpassDependencies.data());

    this->renderPass = 
        this->device->getVkDevice().createRenderPass2(renderPassCreateInfo);

    VulkanDbg::registerVkObjectDbgInfo("The RenderPass", vk::ObjectType::eRenderPass, reinterpret_cast<uint64_t>(vk::RenderPass::CType(this->renderPass)));
}

void RenderPass::createRenderPassBloomDownsample(
    Device& device)
{
    this->device = &device;

    vk::AttachmentDescription2 attachment{};
    attachment.setFormat(PostProcessHandler::HDR_FORMAT);
    attachment.setSamples(vk::SampleCountFlagBits::e1);
    attachment.setLoadOp(vk::AttachmentLoadOp::eClear);
    attachment.setStoreOp(vk::AttachmentStoreOp::eStore);
    attachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
    attachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
    attachment.setInitialLayout(vk::ImageLayout::eUndefined);
    attachment.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

    vk::AttachmentReference2 attachmentReference{};
    attachmentReference.setAttachment(uint32_t(0));
    attachmentReference.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    vk::SubpassDescription2 subpass{};
    subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
    subpass.setColorAttachmentCount(uint32_t(1));
    subpass.setPColorAttachments(&attachmentReference);

    vk::SubpassDependency2 subpassDependecy{};
    subpassDependecy.setSrcSubpass(VK_SUBPASS_EXTERNAL);
    subpassDependecy.setDstSubpass(uint32_t(0));
    subpassDependecy.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput); // Wait until all other graphics have been rendered
    subpassDependecy.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    subpassDependecy.setSrcAccessMask(vk::AccessFlags());
    subpassDependecy.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

    vk::RenderPassCreateInfo2 renderpassCreateinfo{};
    renderpassCreateinfo.setAttachmentCount(uint32_t(1));
    renderpassCreateinfo.setPAttachments(&attachment);
    renderpassCreateinfo.setSubpassCount(uint32_t(1));
    renderpassCreateinfo.setPSubpasses(&subpass);
    renderpassCreateinfo.setDependencyCount(uint32_t(1));
    renderpassCreateinfo.setPDependencies(&subpassDependecy);

    this->renderPass =
        this->device->getVkDevice().createRenderPass2(renderpassCreateinfo);

    VulkanDbg::registerVkObjectDbgInfo("BloomRenderPass", vk::ObjectType::eRenderPass, reinterpret_cast<uint64_t>(vk::RenderPass::CType(this->renderPass)));
}

void RenderPass::createRenderPassBloomUpsample(
    Device& device)
{
    this->device = &device;

    vk::AttachmentDescription2 attachment{};
    attachment.setFormat(PostProcessHandler::HDR_FORMAT);
    attachment.setSamples(vk::SampleCountFlagBits::e1);
    attachment.setLoadOp(vk::AttachmentLoadOp::eLoad);
    attachment.setStoreOp(vk::AttachmentStoreOp::eStore);
    attachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
    attachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
    attachment.setInitialLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
    attachment.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

    vk::AttachmentReference2 attachmentReference{};
    attachmentReference.setAttachment(uint32_t(0));
    attachmentReference.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    vk::SubpassDescription2 subpass{};
    subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
    subpass.setColorAttachmentCount(uint32_t(1));
    subpass.setPColorAttachments(&attachmentReference);

    vk::SubpassDependency2 subpassDependecy{};
    subpassDependecy.setSrcSubpass(VK_SUBPASS_EXTERNAL);
    subpassDependecy.setDstSubpass(uint32_t(0));
    subpassDependecy.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput); // Wait until all other graphics have been rendered
    subpassDependecy.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    subpassDependecy.setSrcAccessMask(vk::AccessFlags());
    subpassDependecy.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

    vk::RenderPassCreateInfo2 renderpassCreateinfo{};
    renderpassCreateinfo.setAttachmentCount(uint32_t(1));
    renderpassCreateinfo.setPAttachments(&attachment);
    renderpassCreateinfo.setSubpassCount(uint32_t(1));
    renderpassCreateinfo.setPSubpasses(&subpass);
    renderpassCreateinfo.setDependencyCount(uint32_t(1));
    renderpassCreateinfo.setPDependencies(&subpassDependecy);

    this->renderPass =
        this->device->getVkDevice().createRenderPass2(renderpassCreateinfo);

    VulkanDbg::registerVkObjectDbgInfo("BloomRenderPass", vk::ObjectType::eRenderPass, reinterpret_cast<uint64_t>(vk::RenderPass::CType(this->renderPass)));
}

void RenderPass::createRenderPassSwapchain(
    Device& device,
    Swapchain& swapchain)
{
    this->device = &device;

    vk::AttachmentDescription2 attachment{};
    attachment.setFormat(swapchain.getVkFormat());
    attachment.setSamples(vk::SampleCountFlagBits::e1);
    attachment.setLoadOp(vk::AttachmentLoadOp::eDontCare);
    attachment.setStoreOp(vk::AttachmentStoreOp::eStore);
    attachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
    attachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
    attachment.setInitialLayout(vk::ImageLayout::eUndefined);
    attachment.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);

    vk::AttachmentReference2 attachmentReference{};
    attachmentReference.setAttachment(uint32_t(0));
    attachmentReference.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    vk::SubpassDescription2 subpass{};
    subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
    subpass.setColorAttachmentCount(uint32_t(1));
    subpass.setPColorAttachments(&attachmentReference);

    vk::SubpassDependency2 subpassDependecy{};
    subpassDependecy.setSrcSubpass(VK_SUBPASS_EXTERNAL);
    subpassDependecy.setDstSubpass(uint32_t(0));
    subpassDependecy.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput); // Wait until all other graphics have been rendered
    subpassDependecy.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    subpassDependecy.setSrcAccessMask(vk::AccessFlags());
    subpassDependecy.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

    vk::RenderPassCreateInfo2 renderpassCreateinfo{};
    renderpassCreateinfo.setAttachmentCount(uint32_t(1));
    renderpassCreateinfo.setPAttachments(&attachment);
    renderpassCreateinfo.setSubpassCount(uint32_t(1));
    renderpassCreateinfo.setPSubpasses(&subpass);
    renderpassCreateinfo.setDependencyCount(uint32_t(1));
    renderpassCreateinfo.setPDependencies(&subpassDependecy);

    this->renderPass =
        this->device->getVkDevice().createRenderPass2(renderpassCreateinfo);

    VulkanDbg::registerVkObjectDbgInfo("SwapchainRenderPass", vk::ObjectType::eRenderPass, reinterpret_cast<uint64_t>(vk::RenderPass::CType(this->renderPass)));
}

void RenderPass::createRenderPassImgui(
    Device& device, 
    Swapchain& swapchain)
{
    this->device = &device;

    vk::AttachmentDescription2 attachment{};
    attachment.setFormat(swapchain.getVkFormat());
    attachment.setSamples(vk::SampleCountFlagBits::e1);
    attachment.setLoadOp(vk::AttachmentLoadOp::eDontCare);
    attachment.setStoreOp(vk::AttachmentStoreOp::eStore);
    attachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
    attachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
    attachment.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal);
    attachment.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

    vk::AttachmentReference2 attachmentReference{};
    attachmentReference.setAttachment(uint32_t(0));
    attachmentReference.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    vk::SubpassDescription2 subpass{};
    subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
    subpass.setColorAttachmentCount(uint32_t(1));
    subpass.setPColorAttachments(&attachmentReference);

    vk::SubpassDependency2 subpassDependecy{};
    subpassDependecy.setSrcSubpass(VK_SUBPASS_EXTERNAL);
    subpassDependecy.setDstSubpass(uint32_t(0));
    subpassDependecy.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput); // Wait until all other graphics have been rendered
    subpassDependecy.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    subpassDependecy.setSrcAccessMask(vk::AccessFlags());
    subpassDependecy.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

    vk::RenderPassCreateInfo2 renderpassCreateinfo{};
    renderpassCreateinfo.setAttachmentCount(uint32_t(1));
    renderpassCreateinfo.setPAttachments(&attachment);
    renderpassCreateinfo.setSubpassCount(uint32_t(1));
    renderpassCreateinfo.setPSubpasses(&subpass);
    renderpassCreateinfo.setDependencyCount(uint32_t(1));
    renderpassCreateinfo.setPDependencies(&subpassDependecy);

    this->renderPass = 
        this->device->getVkDevice().createRenderPass2(renderpassCreateinfo);

    VulkanDbg::registerVkObjectDbgInfo("ImguiRenderPass", vk::ObjectType::eRenderPass, reinterpret_cast<uint64_t>(vk::RenderPass::CType(this->renderPass)));
}

void RenderPass::cleanup()
{
    this->device->getVkDevice().destroyRenderPass(this->renderPass);
}