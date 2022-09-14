#include "Texture.hpp"

#include "PhysicalDevice.hpp"
#include "Device.hpp"

vk::Format Texture::chooseSupportedFormat(
    PhysicalDevice& physicalDevice,
    const std::vector<vk::Format>& formats,
    vk::ImageTiling tiling,
    vk::FormatFeatureFlagBits featureFlags)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    bool is_optimal = false;
    bool is_linear = false;

    // Loop through the options and pick the best suitable one...
    for (auto format : formats)
    {
        // Get Properties for a given format on this device
        vk::FormatProperties2 properties = 
            physicalDevice.getVkPhysicalDevice().getFormatProperties2(format);

        is_linear = (tiling == vk::ImageTiling::eLinear &&                                // Checks whether tiling is set Linear
            (properties.formatProperties.linearTilingFeatures & featureFlags) == featureFlags);  // Checks if the device has the requested LinearTilingFeatures 

        is_optimal = (tiling == vk::ImageTiling::eOptimal &&                              // Checks whether tiling is set Optimal
            (properties.formatProperties.optimalTilingFeatures & featureFlags) == featureFlags); // Checks if the device has the requested OptimalTilingFeatures

        // Depending on choice of Tiling, check different features requirments
        if (is_linear || is_optimal)
        {
            return format;
        }
    }
    throw std::runtime_error("Failed to find a matching format!");
}

vk::Image Texture::createImage(
    VmaAllocator& vma,
    createImageData&& imageData, 
    const std::string& imageDescription)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    // - Create Image - 
    // Image Createion Info
    vk::ImageCreateInfo imageCreateInfo;
    imageCreateInfo.setImageType(vk::ImageType::e2D);         // We will use normal 2D images for everything... (could also use 1D and 3D)
    imageCreateInfo.setExtent(vk::Extent3D(
        imageData.width,    // width of Image extent
        imageData.height,   // height of Image extent
        1                   // depth of Image (Just 1, means no 3D)
    ));
    imageCreateInfo.setMipLevels(uint32_t(1));                        // number of mipmap levels 
    imageCreateInfo.setArrayLayers(uint32_t(1));                        // number of Levels in image array
    imageCreateInfo.setFormat(imageData.format);                   // Format of the image
    imageCreateInfo.setTiling(imageData.tiling);                   // How image data should be "tiled" (arranged for optimal reading speed)
    imageCreateInfo.setInitialLayout(vk::ImageLayout::eUndefined);// Layout of image data on creation
    imageCreateInfo.setUsage(imageData.useFlags);                 // Flags defining how the image will be used
    imageCreateInfo.setSamples(vk::SampleCountFlagBits::e1);    // Number of samples to be used for multi-sampling (1 since we dont use multisampling)
    imageCreateInfo.setSharingMode(vk::SharingMode::eExclusive);// wheter the image will be shared between queues (we will not share images between queues...)

    VmaAllocationCreateInfo vmaAllocCreateInfo{};
    vmaAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;

    vk::Image image;

    // // Create the Image
    if (vmaCreateImage(
        vma, 
        (VkImageCreateInfo*) &imageCreateInfo, 
        &vmaAllocCreateInfo, 
        (VkImage*) &image, 
        imageData.imageMemory, 
        nullptr) != VK_SUCCESS) 
    {
        throw std::runtime_error("Failed to Allocate Image through VMA!");
    }

    return image;
}

vk::ImageView Texture::createImageView(
    Device& device,
	vk::Image image,
	vk::Format format,
	vk::ImageAspectFlags aspectFlags
)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    // Creating a ImageView is similar to how we have the Physical Device and from that create a Logical Device...
    vk::ImageViewCreateInfo viewCreateInfo;

    viewCreateInfo.setImage(image);                           // Image to create view for
    viewCreateInfo.setViewType(vk::ImageViewType::e2D);                           // Type of Image (1D, 2D, 3D, Cube, etc)
    viewCreateInfo.setFormat(format);                             // format of Image Data    
    viewCreateInfo.setComponents(vk::ComponentMapping( // Allows remapping of rgba components to other rgba values!, Identity means they represent themselves
        vk::ComponentSwizzle::eIdentity,    // r
        vk::ComponentSwizzle::eIdentity,    // g
        vk::ComponentSwizzle::eIdentity,    // b
        vk::ComponentSwizzle::eIdentity     // a
    ));

    // Subresources allow the view to view only a Part of a image!
    viewCreateInfo.subresourceRange.aspectMask = aspectFlags;      // Which aspect of image to view (e.g. COLOR_BIT for view)
    /*! Possible aspectMask values are defined with: is vk::ImageAspectFlagBits ...
     * - The 'regular' one is vk::ImageAspectFlagBits::eColor, used for images...
     * */
    viewCreateInfo.subresourceRange.baseMipLevel = 0;                // Which part of the image to view start view from, (a Image can have multiple Mip and Array Layers)...
    viewCreateInfo.subresourceRange.levelCount = 1;                // How many MipMap levels to view, we only view 1 and that will be the "0" referred to by baseMipLevel
    viewCreateInfo.subresourceRange.baseArrayLayer = 0;                // Which BaseArrayLayer to start from, we pick the first: 0
    viewCreateInfo.subresourceRange.layerCount = 1;                // How many layers to check from the baseArrayLayer... (i.e. only view the first layer, layer 0...)

    // Create image view and Return it
    return device.getVkDevice().createImageView(viewCreateInfo);
}