#include "pch.h"
#include <string>

#include "Swapchain.hpp"
#include "VulkanDbg.hpp"
#include "PhysicalDevice.hpp"
#include "Device.hpp"
#include "QueueFamilies.hpp"
#include "../ResTranslator.hpp"
#include "../../application/Window.hpp"
#include "../../dev/Log.hpp"
#include "../Texture.hpp"

vk::SurfaceFormat2KHR Swapchain::chooseBestSurfaceFormat(
    const std::vector<vk::SurfaceFormat2KHR >& formats)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    // "Best" format is subjective.
    // - Format        : vk::Format::eR8G8B8A8Unorm <-- RGBA, all 8 bits (??)
    // -- vk::Format::eB8G8R8A8Unorm is also a possible option... used as 'backup'... (Will be used either if the other choice doesn't exist or this is listed before...
    // - Color Space   : vk::ColorSpaceKHR::eVkColorspaceSrgbNonlinear

    // If the only format in the list is undefined; This means ALL formats are supported!
    if (formats.size() == 1 && 
        formats[0].surfaceFormat.format == vk::Format::eUndefined) 
    {
        return vk::SurfaceFormat2KHR(
            vk::SurfaceFormatKHR(
                { 
                    vk::Format::eR8G8B8A8Unorm,
                    vk::ColorSpaceKHR::eVkColorspaceSrgbNonlinear 
                }
            )
        );
    }

    // If some formats are unavailable, check if the requested formats exist (i.e. vk::Format::eR8G8B8A8Unorm)
    for (const auto& format : formats) 
    {

        if ((format.surfaceFormat.format == vk::Format::eR8G8B8A8Unorm || format.surfaceFormat.format == vk::Format::eB8G8R8A8Unorm) &&
            format.surfaceFormat.colorSpace == vk::ColorSpaceKHR::eVkColorspaceSrgbNonlinear)
        {
            return format;
        }
    }

    Log::error("Could not find a suitable surface format.");

    // If no 'best format' is found, then we return the first format...This is however very unlikely
    return formats[0];
}

vk::PresentModeKHR Swapchain::chooseBestPresentationMode(
    const std::vector<vk::PresentModeKHR>& presentationModes) 
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    
    // The different types we setttle with, first = highest priority
    std::vector<vk::PresentModeKHR> priorityList
    { 
         vk::PresentModeKHR::eMailbox,
         vk::PresentModeKHR::eImmediate,
         vk::PresentModeKHR::eFifo
    };

    for (auto mode : priorityList) 
    {
        if (std::find(presentationModes.begin(), presentationModes.end(), mode) != presentationModes.end())
        {
            // Log::write("Chosen swapchain presentation mode: " + std::to_string((int) mode));

            return mode;
        }
    }

    // Use FIFO as a last resort since it's always guaranteed to be available
    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D Swapchain::chooseBestImageResolution(
    const vk::SurfaceCapabilities2KHR& surfaceCapabilities) 
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    // Since the extents width and height are represented with uint32_t values.
    // Thus we want to make sure that the uin32_t value can represent the size of our surface (??).

    // The default currentExtent.width value of a surface will be the size of the created window...
    // - This is set by the glfwCreateWindowSurface function...
    // - NOTE: the surfaces currentExtent.width/height can change, and then it will not be equal to the window size(??)
    if (surfaceCapabilities.surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
    {
        //IF the current Extent does not vary, then the value will be the same as the windows currentExtent...
        // - This will be the case if the currentExtent.width/height is NOT equal to the maximum value of a uint32_t...
        return surfaceCapabilities.surfaceCapabilities.currentExtent;
    }
    else 
    {
        //IF The current Extent vary, Then currentExtent.width/height will be set to the maximum size of a uint32_t!
        // - This means that we do have to Define it ourself! i.e. grab the size from our glfw_window!
        int width = 0, height = 0;
        this->window->getSize(width, height);

        // Create a new extent using the current window size
        vk::Extent2D newExtent = {};
        newExtent.height = static_cast<uint32_t>(height);     // glfw uses int, but VkExtent2D uses uint32_t...
        newExtent.width = static_cast<uint32_t>(width);

        // Make sure that height/width fetched from the glfw_window is within the max/min height/width of our surface
        // - Do this by clamping the new height and width
        newExtent.width = std::clamp(newExtent.width,
            surfaceCapabilities.surfaceCapabilities.minImageExtent.width,
            surfaceCapabilities.surfaceCapabilities.maxImageExtent.width);
        newExtent.height = std::clamp(newExtent.height,
            surfaceCapabilities.surfaceCapabilities.minImageExtent.height,
            surfaceCapabilities.surfaceCapabilities.maxImageExtent.height);

        return newExtent;
    }
}

void Swapchain::createDepthBuffer()
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    this->depthBufferImage.resize(this->getNumImages());
    this->depthBufferImageMemory.resize(this->getNumImages());
    this->depthBufferImageView.resize(this->getNumImages());

    // Get supported VkFormat for the DepthBuffer
    this->depthFormat = Texture::chooseSupportedFormat(
        *this->physicalDevice,
        {  //Atleast one of these should be available...
            vk::Format::eD32SfloatS8Uint,   // First  Choice: Supported format should be using depth value of 32 bits and using StencilBuffer 8Bits (??)
            vk::Format::eD32Sfloat,           // Second Choice: Supported format shoudl be using depth value of 32 bits
            vk::Format::eD24UnormS8Uint     // third  Choice: Supported format shoudl be using depth value of 24 bits and using StencilBuffer 8Bits (??)
        },
        vk::ImageTiling::eOptimal,                         // We want to use the Optimal Tiling
        vk::FormatFeatureFlagBits::eDepthStencilAttachment); // Make sure the Format supports the Depth Stencil Attatchment Bit....

    // Create one DepthBuffer per Image in the SwapChain
    for (size_t i = 0; i < this->depthBufferImage.size(); i++)
    {
        // Create Depth Buffer Image
        this->depthBufferImage[i] = Texture::createImage(
            *this->vma,
            {
                .width = this->getWidth(),
                .height = this->getHeight(),
                .format = this->depthFormat,
                .tiling = vk::ImageTiling::eOptimal,                        // We want to use Optimal Tiling
                .useFlags = vk::ImageUsageFlagBits::eDepthStencilAttachment     // Image will be used as a Depth Stencil
                            | vk::ImageUsageFlagBits::eInputAttachment,        // Image is local to the device, it will not be changed by the HOST (CPU)
                .imageMemory = &this->depthBufferImageMemory[i]
            },
            "depthBufferImage"
        );

        // Create Depth Buffer Image View
        this->depthBufferImageView[i] = Texture::createImageView(
            *this->device,
            this->depthBufferImage[i],
            depthFormat,
            vk::ImageAspectFlagBits::eDepth
        );
        VulkanDbg::registerVkObjectDbgInfo("depthBufferImageView[" + std::to_string(i) + "]", vk::ObjectType::eImageView, reinterpret_cast<uint64_t>(vk::ImageView::CType(this->depthBufferImageView[i])));
        VulkanDbg::registerVkObjectDbgInfo("depthBufferImage[" + std::to_string(i) + "]", vk::ObjectType::eImage, reinterpret_cast<uint64_t>(vk::Image::CType(this->depthBufferImage[i])));
    }
}

Swapchain::Swapchain()
    : swapchain(VK_NULL_HANDLE),
    window(nullptr),
    physicalDevice(nullptr),
    device(nullptr),
    surface(nullptr),
    queueFamilies(nullptr),
    vma(nullptr),
    numMinimumImages(0)
{
}

Swapchain::~Swapchain()
{
}

void Swapchain::createSwapchain(
    Window& window,
    PhysicalDevice& physicalDevice,
    Device& device,
    vk::SurfaceKHR& surface,
    QueueFamilies& queueFamilies,
    VmaAllocator& vma)
{
    {
    #ifndef VENGINE_NO_PROFILING
        ZoneScoped; //:NOLINT
    #endif

        this->window = &window;
        this->physicalDevice = &physicalDevice;
        this->device = &device;
        this->surface = &surface;
        this->queueFamilies = &queueFamilies;
        this->vma = &vma;

        Swapchain::getDetails(
            this->physicalDevice->getVkPhysicalDevice(),
            *this->surface,
            this->swapchainDetails
        );

        // Store Old Swapchain, if it exists
        vk::SwapchainKHR oldSwapchain = this->swapchain;

        //Find 'optimal' surface values for our swapChain
        // - 1. Choose best surface Format
        vk::SurfaceFormat2KHR  surfaceFormat =
            this->chooseBestSurfaceFormat(this->swapchainDetails.Format);

        // - 2. Choose best presentation Mode
        vk::PresentModeKHR presentationMode =
            this->chooseBestPresentationMode(this->swapchainDetails.presentationMode);

        // - 3. Choose Swap Chain image Resolution
        vk::Extent2D imageExtent =
            this->chooseBestImageResolution(this->swapchainDetails.surfaceCapabilities);

        this->numMinimumImages = this->swapchainDetails.surfaceCapabilities.surfaceCapabilities.minImageCount;

        // --- PREPARE DATA FOR SwapChainCreateInfo ... ---
        // Minimum number of images our swapChain should use.
        // - By setting the minImageCount to 1 more image than the amount defined in surfaceCapabilities we enable Triple Buffering!
        // - NOTE: we store the 'minImageCount+1' in a variable, we need to check that 'minImageCount+1' is not more than 'maxImageCount'!
        uint32_t imageCount = std::clamp(
            this->swapchainDetails.surfaceCapabilities.surfaceCapabilities.minImageCount + 1,
            this->swapchainDetails.surfaceCapabilities.surfaceCapabilities.minImageCount,
            this->swapchainDetails.surfaceCapabilities.surfaceCapabilities.maxImageCount);

        if (imageCount == 0)
        {
            // if swapChainDetails.surfaceCapabilities.maxImageCount was 0 then imageCount will now be 0 too.
            // This CAN happen IF there is no limit on how many images we can store in the SwapChain.
            // - i.e. maxImageCount == 0, then there is no maxImageCount!
            //imageCount    = swapChainDetails.surfaceCapabilities.minImageCount + 1; //!! Nope
            imageCount = this->swapchainDetails.surfaceCapabilities.surfaceCapabilities.maxImageCount; // (??)
            /*! We use the max image count if we can, the clamping we did *Seems* to ensure that we don't get a imageCount is not 0...
             * I'm not sure if I'm missing something... this seems redundant.
             * Could I just :
             *  imageCount = (maxImageCount != 0) ? maxImageCount : minImageCount +1 ;
             *
             * Also... Do I always want to use the MaxImageCount just because I can? Or is minImageCount + X a better choice...
             * */

            Log::warning("Swapchain image count was set to 0.");
        }

        //Create the SwapChain Create Info!
        vk::SwapchainCreateInfoKHR  swapChainCreateInfo = {};
        swapChainCreateInfo.setSurface(surface);
        swapChainCreateInfo.setImageFormat(surfaceFormat.surfaceFormat.format);
        swapChainCreateInfo.setImageColorSpace(surfaceFormat.surfaceFormat.colorSpace);
        swapChainCreateInfo.setPresentMode(presentationMode);
        swapChainCreateInfo.setImageExtent(imageExtent);
        swapChainCreateInfo.setMinImageCount(uint32_t(imageCount));
        swapChainCreateInfo.setImageArrayLayers(uint32_t(1));                                    // Numbers of layers for each image in chain
        swapChainCreateInfo.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment              // What 'Attachment' are drawn to the image
            | vk::ImageUsageFlagBits::eTransferSrc);    // Tells us we want to be able to use the image, if we want to take a screenshot etc...    

        /*! The imageUsage defines how the image is intended to be used, there's a couple different options.
         * - vk::ImageUsageFlagBits::eColorAttachment        : Used for Images with colors (??)
         * NOTE: Generally always vk::ImageUsageFlagBits::eColorAttachment...
         *       This is because the SwapChain is used to present images to the screen,
         *       if we wanted to draw a depthBuffer onto the screen, then we could specify another imageUsage flag... (??)
         * */
        swapChainCreateInfo.preTransform = this->swapchainDetails.surfaceCapabilities.surfaceCapabilities.currentTransform;
        swapChainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;    // Draw as is, opaque...
        swapChainCreateInfo.clipped = VK_TRUE;                              // dont draw not visible parts of window

         // We pick mode based on if the GraphicsQueue and PresentationQueue is the same queue...
        int32_t graphicsQueueIndex = queueFamilies.getGraphicsIndex();
        int32_t presentQueueIndex = queueFamilies.getPresentIndex();
        std::array<uint32_t, 2> queueFamilyArray
        {
            static_cast<uint32_t>(graphicsQueueIndex),
            static_cast<uint32_t>(presentQueueIndex)
        };

        // If Graphics and Presentation families are different, then SwapChain must let images be shared between families!
        if (graphicsQueueIndex != presentQueueIndex) 
        {
            swapChainCreateInfo.setImageSharingMode(vk::SharingMode::eConcurrent);   // Use Concurrent mode if more than 1 family is using the swapchain
            swapChainCreateInfo.setQueueFamilyIndexCount(uint32_t(2));                            // How many different queue families will use the swapchain
            swapChainCreateInfo.setPQueueFamilyIndices(queueFamilyArray.data());         // Array containing the queues that will share images
        }
        else 
        {
            swapChainCreateInfo.setImageSharingMode(vk::SharingMode::eExclusive);    // Use Exclusive mode if only one Queue Family uses the SwapChain...
            swapChainCreateInfo.setQueueFamilyIndexCount(uint32_t(0));        // Note; this is the default value
            swapChainCreateInfo.setPQueueFamilyIndices(nullptr);  // Note; this is the default value
        }


        /*! If we want to create a new SwapChain, this would be needed when for example resizing the window.
         *  with the oldSwapchain we can pass the old swapChains responsibility to the new SwapChain...
         * */
         // IF old swapChain been destroyed and this one replaces it, then link old one to quickly hand over responsibilities...
        swapChainCreateInfo.setOldSwapchain(oldSwapchain); // VK_NULL_HANDLE on initialization, previous all other times...

        // Create The SwapChain!    
        this->swapchain = this->device->getVkDevice().createSwapchainKHR(swapChainCreateInfo);
        VulkanDbg::registerVkObjectDbgInfo("Swapchain", vk::ObjectType::eSwapchainKHR, reinterpret_cast<uint64_t>(vk::SwapchainKHR::CType(this->swapchain)));

        // Store both the VkExtent2D and VKFormat, so they can easily be used later...
        this->swapchainImageFormat = surfaceFormat.surfaceFormat.format;
        this->swapchainExtent = imageExtent;

        // Get all Images from the SwapChain and store them in our swapChainImages Vector...
        this->swapchainImages =
            this->device->getVkDevice().getSwapchainImagesKHR(this->swapchain);

        this->swapchainImageViews.resize(this->swapchainImages.size());
        for (size_t i = 0; i < this->swapchainImages.size(); ++i)
        {
            // Create the Image View
            this->swapchainImageViews[i] = Texture::createImageView(
                *this->device,
                this->swapchainImages[i],
                this->swapchainImageFormat,
                vk::ImageAspectFlagBits::eColor
            );
            VulkanDbg::registerVkObjectDbgInfo("Swapchain_ImageView[" + std::to_string(i) + "]", vk::ObjectType::eImageView, reinterpret_cast<uint64_t>(vk::ImageView::CType(this->swapchainImageViews[i])));
            VulkanDbg::registerVkObjectDbgInfo("Swapchain_Image[" + std::to_string(i) + "]", vk::ObjectType::eImage, reinterpret_cast<uint64_t>(vk::Image::CType(this->swapchainImages[i])));
        }

        if (oldSwapchain)
        {
            this->device->getVkDevice().destroySwapchainKHR(oldSwapchain);
        }

        this->createDepthBuffer();

        // Update resolution translator
        ResTranslator::updateWindowSize(
            static_cast<uint32_t>(imageExtent.width),
            static_cast<uint32_t>(imageExtent.height)
        );
    }
}

void Swapchain::createFramebuffers(vk::RenderPass& renderPass)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    // Resize framebuffer count to equal swap chain image count
    this->swapchainFrameBuffers.resize(this->getNumImages());

    // Create a framebuffer for each swap chain image
    for (size_t i = 0; i < this->swapchainFrameBuffers.size(); i++) 
    {
        std::array<vk::ImageView, 2> attachments = 
        {
                this->swapchainImageViews[i],
                this->depthBufferImageView[i]
        };

        vk::FramebufferCreateInfo framebufferCreateInfo;
        framebufferCreateInfo.setRenderPass(renderPass);                                      // Render pass layout the framebuyfffeer will be used with
        framebufferCreateInfo.setAttachmentCount(static_cast<uint32_t>(attachments.size()));
        framebufferCreateInfo.setPAttachments(attachments.data());                            // List of attatchemnts (1:1 with render pass)
        framebufferCreateInfo.setWidth(this->swapchainExtent.width);
        framebufferCreateInfo.setHeight(this->swapchainExtent.height);
        framebufferCreateInfo.setLayers(uint32_t(1));

        this->swapchainFrameBuffers[i] = device->getVkDevice().createFramebuffer(framebufferCreateInfo);
        VulkanDbg::registerVkObjectDbgInfo("SwapchainFramebuffer[" + std::to_string(i) + "]", vk::ObjectType::eFramebuffer, reinterpret_cast<uint64_t>(vk::Framebuffer::CType(this->swapchainFrameBuffers[i])));
    }
}

void Swapchain::recreateSwapchain(vk::RenderPass& renderPass)
{
    this->cleanup(false);

    this->createSwapchain(
        *this->window, 
        *this->physicalDevice,
        *this->device, 
        *this->surface, 
        *this->queueFamilies,
        *this->vma
    );

    this->createFramebuffers(renderPass);
}

void Swapchain::getDetails(
    vk::PhysicalDevice& physDevice, 
    vk::SurfaceKHR& surface, 
    SwapchainDetails& outputDetails)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    outputDetails = {};

    // Surface capabilities
    vk::PhysicalDeviceSurfaceInfo2KHR physicalDeviceSurfaceInfo{};
    physicalDeviceSurfaceInfo.setSurface(surface);
    outputDetails.surfaceCapabilities =
        physDevice.getSurfaceCapabilities2KHR(
            physicalDeviceSurfaceInfo
        );

    // Formats
    outputDetails.Format =
        physDevice.getSurfaceFormats2KHR(
            physicalDeviceSurfaceInfo
        );

    // Presentation modes
    outputDetails.presentationMode =
        physDevice.getSurfacePresentModesKHR(
            surface
        );
}

void Swapchain::cleanup(bool destroySwapchain)
{
    // Depth
    for (size_t i = 0; i < this->depthBufferImage.size(); i++)
    {
        this->device->getVkDevice().destroyImageView(this->depthBufferImageView[i]);
        this->device->getVkDevice().destroyImage(this->depthBufferImage[i]);
        vmaFreeMemory(*this->vma, this->depthBufferImageMemory[i]);
    }

    // Swapchain image views
    for (auto view : this->swapchainImageViews)
    {
        this->device->getVkDevice().destroyImageView(view);
    }
    this->swapchainImageViews.resize(0);

    // The swapchain destruction will destroy the swapchain images
    this->swapchainImages.resize(0);

    // Swapchain
    if (destroySwapchain)
    {
        this->device->getVkDevice().destroySwapchainKHR(this->swapchain);
    }

    // Framebuffers
    for (auto framebuffer : this->swapchainFrameBuffers)
    {
        this->device->getVkDevice().destroyFramebuffer(framebuffer);
    }
    this->swapchainFrameBuffers.resize(0);
}

bool Swapchain::canCreateValidSwapchain()
{
    // Get details
    SwapchainDetails swapchainDetails{};
    Swapchain::getDetails(
        this->physicalDevice->getVkPhysicalDevice(),
        *this->surface,
        swapchainDetails
    );

    // Make sure the size is larger than 0
    return swapchainDetails.surfaceCapabilities.surfaceCapabilities.maxImageExtent.width > 0 ||
        swapchainDetails.surfaceCapabilities.surfaceCapabilities.maxImageExtent.height > 0;
}