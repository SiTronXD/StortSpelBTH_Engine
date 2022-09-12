#include "Swapchain.hpp"
#include "VulkanDbg.hpp"
#include "Device.hpp"

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

    // If no 'best format' is found, then we return the first format...This is however very unlikely
    return formats[0];
}

vk::PresentModeKHR Swapchain::chooseBestPresentationMode(const std::vector<vk::PresentModeKHR>& presentationModes) {
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    return vk::PresentModeKHR::eImmediate;
    // "Best" PresentationMode is subjective. Here we pick 'VSYNC'...
    std::vector<vk::PresentModeKHR> priorityList{ // The different types we setttle with, first = highset priority
         vk::PresentModeKHR::eMailbox,
         vk::PresentModeKHR::eImmediate,
         vk::PresentModeKHR::eFifo
    };

    for (auto prioritized_mode : priorityList) 
    {

        if (std::find(presentationModes.begin(), presentationModes.end(), prioritized_mode) != presentationModes.end()) 
        {
            return prioritized_mode;
        }
    }

    // If Mailbox Mode does not exist, use FIFO since it always should be available...
    return vk::PresentModeKHR::eFifo;
}

void Swapchain::recreateCleanup()
{
    for (auto image : this->swapchainImages) {
        device->getVkDevice().destroyImageView(image.imageView);
    }
    this->swapchainImages.resize(0);

    for (auto framebuffer : this->swapchainFrameBuffers) {
        this->device->getVkDevice().destroyFramebuffer(framebuffer);
    }
    this->swapchainFrameBuffers.resize(0);
}

Swapchain::Swapchain()
    : swapchain(VK_NULL_HANDLE),
    device(nullptr)
{
}

Swapchain::~Swapchain()
{
}

void Swapchain::createSwapchain(Device& device)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    this->device = &device;

    // Store Old Swapchain, if it exists
    vk::SwapchainKHR oldSwapchain = this->swapchain;

    //Find 'optimal' surface values for our swapChain
    // - 1. Choose best surface Format
    vk::SurfaceFormat2KHR  surfaceFormat = 
        this->chooseBestSurfaceFormat(this->swapchainDetails.Format);

    // - 2. Choose best presentation Mode
    vk::PresentModeKHR presentationMode = 
        this->chooseBestPresentationMode(this->swapchainDetails.presentationMode);

    // - 3. Chose Swap Chain image Resolution
    vk::Extent2D imageExtent = 
        this->chooseBestImageResolution(this->swapchainDetails.surfaceCapabilities);

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
    }

    //Create the SwapChain Create Info!
    vk::SwapchainCreateInfoKHR  swapChainCreateInfo = {};
    swapChainCreateInfo.setSurface(this->surface);
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

    //! -- THE TWO DIFFERENT QUEUES - Graphics queue and Presentation Queue --
    /*! The Graphics Queue      : will draw the images contained in our swapChain
     *  The Presentation Queue  : will present the images to the surface (i.e. screen/window)
     *
     *  There are 2 Different modes the swapChain can interact with these Queues;
     *  - Exclusive interaction     : A unique Image from the SwapChain can only be interacted with one queue at the same time
     *  - Concurrent interaction    : A unique Image from the SwapChain can only be interacted with multiple queues at the same time
     *
     *  NOTE: Concurrent interaction is typically slower (for 2 reasons)!
     *  - The Concurrent mode have more overhead
     *  - IF the GraphicsQueue AND PresentationQueue is the SAME queue, then there is no need for multiple queues to interact with the image...
     * */

     // We pick mode based on if the GraphicsQueue and PresentationQueue is the same queue...
     //TODO: the QueueFamilyIndices should be stored somewhere rather than fetched again...
    std::array<uint32_t, 2> queueFamilies
    { 
        static_cast<uint32_t>(this->queueFamilies.graphicsFamily),
        static_cast<uint32_t>(this->queueFamilies.presentationFamily) 
    };

    // If Graphics and Presentation families are different, then SwapChain must let images be shared between families!
    if (this->queueFamilies.graphicsFamily != this->queueFamilies.presentationFamily) {

        swapChainCreateInfo.setImageSharingMode(vk::SharingMode::eConcurrent);   // Use Concurrent mode if more than 1 family is using the swapchain
        swapChainCreateInfo.setQueueFamilyIndexCount(uint32_t(2));                            // How many different queue families will use the swapchain
        swapChainCreateInfo.setPQueueFamilyIndices(queueFamilies.data());         // Array containing the queues that will share images
    }
    else {
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
    VulkanDbg::registerVkObjectDbgInfo("Swapchain", vk::ObjectType::eSwapchainKHR, reinterpret_cast<uint64_t>(vk::SwapchainKHR::CType(this->swapChain)));

    /*! REMEMBER:
     * All of Vulkans functions labeled with "Create" creates something that will need to be destroyed! ...
     * */

     // Store both the VkExtent2D and VKFormat, so they can easily be used later...
    this->swapchainImageFormat = surfaceFormat.surfaceFormat.format; // We need this to create a ImageView...
    this->swapchainExtent = imageExtent;          // We need this to create a ImageView...
    /*! ImageView is the interface which we manage image through. (interface to the image... sort of (??))
     * - With an ImageView we can View the Image...
     * */

     // Get all Images from the SwapChain and store them in our swapChainImages Vector...
    std::vector<vk::Image> images = 
        this->device->getVkDevice().getSwapchainImagesKHR(this->swapchain);

    uint32_t index = 0;
    for (vk::Image image : images) 
    {
        // Copy the Image Handle ...
        SwapChainImage swapChainImage = {};
        swapChainImage.image = image;

        // Create the Image View
        swapChainImage.imageView = createImageView(
            image, 
            this->swapchainImageFormat, 
            vk::ImageAspectFlagBits::eColor
        );
        VulkanDbg::registerVkObjectDbgInfo("Swapchain_ImageView[" + std::to_string(index) + "]", vk::ObjectType::eImageView, reinterpret_cast<uint64_t>(vk::ImageView::CType(swapChainImage.imageView)));
        VulkanDbg::registerVkObjectDbgInfo("Swapchain_Image[" + std::to_string(index) + "]", vk::ObjectType::eImage, reinterpret_cast<uint64_t>(vk::Image::CType(swapChainImage.image)));

        index++;
        this->swapchainImages.push_back(swapChainImage);
    }

    if (oldSwapchain) 
    {
        this->device->getVkDevice().destroySwapchainKHR(oldSwapchain);
    }
}

void Swapchain::createFramebuffers()
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    // Resize framebuffer count to equal swap chain image count
    this->swapchainFrameBuffers.resize(this->getNumImages());

    // Create a framebuffer for each swap chain image
    for (size_t i = 0; i < this->swapchainFrameBuffers.size(); i++) {

        std::array<vk::ImageView, 3> attachments = {
                this->swapchainImages[i].imageView,   // Attatchment on index 0 of array : swapchain image
                this->colorBufferImageView[i],   // Attachement on index 1 of array : color
                this->depthBufferImageView[i]  // Attatchment on index 2 of array : depth
        };

        vk::FramebufferCreateInfo framebufferCreateInfo;
        framebufferCreateInfo.setRenderPass(renderPass_base);                                      // Render pass layout the framebuyfffeer will be used with
        framebufferCreateInfo.setAttachmentCount(static_cast<uint32_t>(attachments.size()));
        framebufferCreateInfo.setPAttachments(attachments.data());                            // List of attatchemnts (1:1 with render pass)
        framebufferCreateInfo.setWidth(this->swapchainExtent.width);
        framebufferCreateInfo.setHeight(this->swapchainExtent.height);
        framebufferCreateInfo.setLayers(uint32_t(1));

        this->swapchainFrameBuffers[i] = device->getVkDevice().createFramebuffer(framebufferCreateInfo);
        VulkanDbg::registerVkObjectDbgInfo("SwapchainFramebuffer[" + std::to_string(i) + "]", vk::ObjectType::eFramebuffer, reinterpret_cast<uint64_t>(vk::Framebuffer::CType(this->swapChainFrameBuffers[i])));
    }
}

void Swapchain::recreateSwapchain()
{
}

void Swapchain::cleanup()
{
    this->recreateCleanup();

    this->device->getVkDevice().destroySwapchainKHR(this->swapchain);
}
