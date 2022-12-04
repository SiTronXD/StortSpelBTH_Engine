#include "pch.h"
#include "Swapchain.hpp"
#include "RenderPass.hpp"
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

        // Create a new(__FILE__, __LINE__) extent using the current window size
        vk::Extent2D newExtent = {};
        newExtent.height = static_cast<uint32_t>(height);     // glfw uses int, but VkExtent2D uses uint32_t...
        newExtent.width = static_cast<uint32_t>(width);

        // Make sure that height/width fetched from the glfw_window is within the max/min height/width of our surface
        // - Do this by clamping the new(__FILE__, __LINE__) height and width
        newExtent.width = std::clamp(newExtent.width,
            surfaceCapabilities.surfaceCapabilities.minImageExtent.width,
            surfaceCapabilities.surfaceCapabilities.maxImageExtent.width);
        newExtent.height = std::clamp(newExtent.height,
            surfaceCapabilities.surfaceCapabilities.minImageExtent.height,
            surfaceCapabilities.surfaceCapabilities.maxImageExtent.height);

        return newExtent;
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
            this->details
        );

        // Store Old Swapchain, if it exists
        vk::SwapchainKHR oldSwapchain = this->swapchain;

        //Find 'optimal' surface values for our swapChain
        // - 1. Choose best surface Format
        vk::SurfaceFormat2KHR  surfaceFormat =
            this->chooseBestSurfaceFormat(this->details.Format);

        // - 2. Choose best presentation Mode
        vk::PresentModeKHR presentationMode =
            this->chooseBestPresentationMode(this->details.presentationMode);

        // - 3. Choose Swap Chain image Resolution
        vk::Extent2D imageExtent =
            this->chooseBestImageResolution(this->details.surfaceCapabilities);

        this->numMinimumImages = this->details.surfaceCapabilities.surfaceCapabilities.minImageCount;

        // --- PREPARE DATA FOR SwapChainCreateInfo ... ---
        // Minimum number of images our swapChain should use.
        // - By setting the minImageCount to 1 more image than the amount defined in surfaceCapabilities we enable Triple Buffering!
        // - NOTE: we store the 'minImageCount+1' in a variable, we need to check that 'minImageCount+1' is not more than 'maxImageCount'!
        uint32_t imageCount = std::clamp(
            this->details.surfaceCapabilities.surfaceCapabilities.minImageCount + 1,
            this->details.surfaceCapabilities.surfaceCapabilities.minImageCount,
            this->details.surfaceCapabilities.surfaceCapabilities.maxImageCount);

        if (imageCount == 0)
        {
            // if swapChainDetails.surfaceCapabilities.maxImageCount was 0 then imageCount will now be 0 too.
            // This CAN happen IF there is no limit on how many images we can store in the SwapChain.
            // - i.e. maxImageCount == 0, then there is no maxImageCount!
            //imageCount    = swapChainDetails.surfaceCapabilities.minImageCount + 1; //!! Nope
            imageCount = this->details.surfaceCapabilities.surfaceCapabilities.maxImageCount; // (??)
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
        swapChainCreateInfo.preTransform = this->details.surfaceCapabilities.surfaceCapabilities.currentTransform;
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


        /*! If we want to create a new(__FILE__, __LINE__) SwapChain, this would be needed when for example resizing the window.
         *  with the oldSwapchain we can pass the old swapChains responsibility to the new(__FILE__, __LINE__) SwapChain...
         * */
         // IF old swapChain been destroyed and this one replaces it, then link old one to quickly hand over responsibilities...
        swapChainCreateInfo.setOldSwapchain(oldSwapchain); // VK_NULL_HANDLE on initialization, previous all other times...

        // Create The SwapChain!    
        this->swapchain = this->device->getVkDevice().createSwapchainKHR(swapChainCreateInfo);
        VulkanDbg::registerVkObjectDbgInfo("Swapchain", vk::ObjectType::eSwapchainKHR, reinterpret_cast<uint64_t>(vk::SwapchainKHR::CType(this->swapchain)));

        // Store both the VkExtent2D and VKFormat, so they can easily be used later...
        this->imageFormat = surfaceFormat.surfaceFormat.format;
        this->extent = imageExtent;

        // Get all Images from the SwapChain and store them in our swapChainImages Vector...
        this->images =
            this->device->getVkDevice().getSwapchainImagesKHR(this->swapchain);

        this->imageViews.resize(this->images.size());
        for (size_t i = 0; i < this->images.size(); ++i)
        {
            // Create the Image View
            this->imageViews[i] = Texture::createImageView(
                *this->device,
                this->images[i],
                this->imageFormat,
                vk::ImageAspectFlagBits::eColor
            );
            VulkanDbg::registerVkObjectDbgInfo("Swapchain_ImageView[" + std::to_string(i) + "]", vk::ObjectType::eImageView, reinterpret_cast<uint64_t>(vk::ImageView::CType(this->imageViews[i])));
            VulkanDbg::registerVkObjectDbgInfo("Swapchain_Image[" + std::to_string(i) + "]", vk::ObjectType::eImage, reinterpret_cast<uint64_t>(vk::Image::CType(this->images[i])));
        }

        // Destroy old swapchain
        if (oldSwapchain)
        {
            this->device->getVkDevice().destroySwapchainKHR(oldSwapchain);
        }

        // Update resolution translator
        ResTranslator::updateWindowSize(
            static_cast<uint32_t>(imageExtent.width),
            static_cast<uint32_t>(imageExtent.height)
        );
    }
}

void Swapchain::createFramebuffers(RenderPass& renderPass)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    
    std::vector<std::vector<vk::ImageView>> framebufferAttachments(this->getNumImages());
    for (size_t i = 0; i < framebufferAttachments.size(); ++i)
    {
        framebufferAttachments[i].push_back(this->getImageView(i));
    }
    this->framebuffers.create(
        *this->device,
        renderPass,
        this->getVkExtent(),
        framebufferAttachments
    );
}

void Swapchain::recreateSwapchain(RenderPass& renderPass)
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
    // Swapchain image views
    for (auto view : this->imageViews)
    {
        this->device->getVkDevice().destroyImageView(view);
    }
    this->imageViews.clear();

    // Swapchain destruction will destroy the swapchain images
    // for us.
    this->images.clear();

    // Swapchain
    if (destroySwapchain)
    {
        this->device->getVkDevice().destroySwapchainKHR(this->swapchain);
    }

    // Framebuffers
    this->framebuffers.cleanup();
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