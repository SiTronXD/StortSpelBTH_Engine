#include "Texture.hpp"

#include "Device.hpp"

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