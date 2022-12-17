#include "pch.h"
#include "Texture.hpp"
#include "vulkan/PhysicalDevice.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/CommandBuffer.hpp"

Texture::Texture() 
    : physicalDevice(nullptr),
    device(nullptr), 
    vma(nullptr),
    imageMemory(nullptr),
    width(0),
    height(0),
    textureSamplerIndex(~0u),
    descriptorIndex(~0u)
{ }

Texture::~Texture()
{ }

void Texture::createAsDepthTexture(
    PhysicalDevice& physicalDevice,
    Device& device,
    VmaAllocator& vma,
    const uint32_t& width,
    const uint32_t& height,
    const uint32_t& arrayLayers,
    const uint32_t& textureSamplerIndex,
    const vk::ImageUsageFlagBits& extraUsageFlags)
{
    this->physicalDevice = &physicalDevice;
    this->device = &device;
    this->vma = &vma;
    this->width = width;
    this->height = height;
    this->textureSamplerIndex = textureSamplerIndex;

    // Get supported VkFormat for the depth buffer
    this->format = Texture::getDepthBufferFormat(physicalDevice);

    // Image
    this->image = Texture::createImage(
        *this->vma,
        {
            .width = this->getWidth(),
            .height = this->getHeight(),
            .arrayLayers = arrayLayers,
            .mipLevels = 1,
            .format = this->format,
            .tiling = vk::ImageTiling::eOptimal,                        // We want to use Optimal Tiling
            .useFlags = vk::ImageUsageFlagBits::eDepthStencilAttachment
                | extraUsageFlags,
            .imageMemory = &this->imageMemory
        }
    );

    // Array layer image views
    for (uint32_t i = 0; i < arrayLayers; ++i)
    {
        this->layerImageViews.push_back(
            Texture::createImageView(
                *this->device,
                this->image,
                this->format,
                vk::ImageAspectFlagBits::eDepth,
                vk::ImageViewType::e2DArray,
                arrayLayers,
                i
            )
        );
    }

    // Regular image view
    this->entireImageView =
        Texture::createImageView(
            *this->device,
            this->image,
            this->format,
            vk::ImageAspectFlagBits::eDepth,
            vk::ImageViewType::e2DArray,
            arrayLayers,
            0,
            true
        );
}

void Texture::create(
    PhysicalDevice& physicalDevice,
    Device& device,
    VmaAllocator& vma,
    vk::Queue& transferQueue,
    vk::CommandPool& transferCommandPool,
    stbi_uc* imageData,
    const uint32_t& width,
    const uint32_t& height,
    const TextureSettings& textureSettings,
    const uint32_t& textureSamplerIndex)
{
    this->physicalDevice = &physicalDevice;
    this->device = &device;
    this->vma = &vma;
    this->width = width;
    this->height = height;
    this->textureSamplerIndex = textureSamplerIndex;

    this->format = vk::Format::eR8G8B8A8Unorm;

    // Save the pixels only if it should keep the info
    if (textureSettings.keepCpuPixelInfo)
    {
        // Loop through pixels in 1D array
        this->pixels.reserve(width * height);
        for (uint32_t i = 0, size = width * height; i < size; ++i)
        {
            // Image data index
            uint32_t imageDataIndex = i * 4;

            // Create pixel
            Pixel pixel{};
            pixel.r = imageData[imageDataIndex + 0];
            pixel.g = imageData[imageDataIndex + 1];
            pixel.b = imageData[imageDataIndex + 2];
            pixel.a = imageData[imageDataIndex + 3];

            // Add pixel
            this->pixels.push_back(pixel);
        }
    }

    vk::DeviceSize imageSize = width * height * 4;

    // Create staging buffer for the texture
    vk::Buffer imageStagingBuffer = nullptr;
    VmaAllocation imageStagingBufferMemory = nullptr;
    VmaAllocationInfo allocInfo;
    Buffer::createBuffer(
        {
        .bufferSize = imageSize,
        .bufferUsageFlags = vk::BufferUsageFlagBits::eTransferSrc,
        .bufferAllocationFlags =
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
            VMA_ALLOCATION_CREATE_MAPPED_BIT |
            VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT, // Dedicated memory can be completely freed, and therefore saves cpu RAM afterwards.
        .buffer = &imageStagingBuffer,
        .bufferMemory = &imageStagingBufferMemory,
        .allocationInfo = &allocInfo,
        .vma = this->vma 
        }
    );

    // Map staging buffer
    void* data = nullptr;
    if (vmaMapMemory(*this->vma, imageStagingBufferMemory,
        &data) != VK_SUCCESS) {
        throw std::runtime_error(
            "Failed to allocate Mesh Staging Texture Image Buffer Using VMA!");
    };

    // Copy data to staging buffer
    memcpy(data, imageData, imageSize);

    // Unmap stagning buffer
    vmaUnmapMemory(*this->vma, imageStagingBufferMemory);

    // Create image
    this->image = Texture::createImage(
        *this->vma,
        { 
            .width = static_cast<uint32_t>(width),
            .height = static_cast<uint32_t>(height),
            .arrayLayers = 1,
            .mipLevels = 1,
            .format = this->format, 
            .tiling = vk::ImageTiling::eOptimal,
            .useFlags = vk::ImageUsageFlagBits::eTransferDst
                | vk::ImageUsageFlagBits::eSampled,
            .imageMemory = &this->imageMemory
        }
    );

    // Copy data to image
    Texture::transitionImageLayout(
        *this->device,
        transferQueue, // Same as graphics Queue
        transferCommandPool,
        this->image,                    // Image to transition the layout on
        vk::ImageLayout::eUndefined, // Image Layout to transition the image from
        vk::ImageLayout::eTransferDstOptimal
    ); 

    // Copy Data to image
    Buffer::copyBufferToImage(
        this->device->getVkDevice(),
        transferQueue,
        transferCommandPool, 
        imageStagingBuffer,
        this->image, 
        this->width, 
        this->height
    );

    // Transition iamge to be shader readable for shader usage
    Texture::transitionImageLayout(
        *this->device,
        transferQueue,
        transferCommandPool,
        this->image,
        vk::ImageLayout::eTransferDstOptimal,
        vk::ImageLayout::eShaderReadOnlyOptimal
    );

    // Deallocate staging buffer
    this->device->getVkDevice().destroyBuffer(
        imageStagingBuffer);
    vmaFreeMemory(*this->vma, imageStagingBufferMemory);

    // Image view
    this->entireImageView =
        Texture::createImageView(
            *this->device,
            this->image,
            this->format,
            vk::ImageAspectFlagBits::eColor,
            vk::ImageViewType::e2D,
            1,
            0,
            true
        );
}

void Texture::createRenderableTexture(
    PhysicalDevice& physicalDevice,
    Device& device,
    VmaAllocator& vma,
    vk::Queue& transferQueue,
    vk::CommandPool& transferCommandPool,
    const vk::Format& format,
    const uint32_t& width,
    const uint32_t& height,
    const uint32_t& mipLevels,
    const uint32_t& textureSamplerIndex,
    const vk::ImageUsageFlagBits& extraUsageFlags)
{
    this->physicalDevice = &physicalDevice;
    this->device = &device;
    this->vma = &vma;
    this->width = width;
    this->height = height;
    this->textureSamplerIndex = textureSamplerIndex;

    // Get supported VkFormat for the depth buffer
    this->format = format;

    // Image
    this->image = Texture::createImage(
        *this->vma,
        {
            .width = this->getWidth(),
            .height = this->getHeight(),
            .arrayLayers = 1,
            .mipLevels = mipLevels,
            .format = this->format,
            .tiling = vk::ImageTiling::eOptimal,                        // We want to use Optimal Tiling
            .useFlags = vk::ImageUsageFlagBits::eColorAttachment
                | extraUsageFlags,
            .imageMemory = &this->imageMemory
        }
    );

    // Mip image views
    for (uint32_t i = 0; i < mipLevels; ++i)
    {
        this->mipImageViews.push_back(
            Texture::createImageView(
                *this->device,
                this->image,
                this->format,
                vk::ImageAspectFlagBits::eColor,
                vk::ImageViewType::e2D,
                1,
                0,
                true,
                mipLevels,
                i
            )
        );
    }

    // Image view
    this->entireImageView =
        Texture::createImageView(
            *this->device,
            this->image,
            this->format,
            vk::ImageAspectFlagBits::eColor,
            vk::ImageViewType::e2D,
            1,
            0,
            true
        );
}

void Texture::setDescriptorIndex(const uint32_t& descriptorIndex)
{
    this->descriptorIndex = descriptorIndex;
}

void Texture::cleanup() 
{
    // Regular image view
    this->device->getVkDevice().destroyImageView(this->entireImageView);

    // Layer image views
    for (auto& imageView : this->layerImageViews)
    {
        this->device->getVkDevice().destroyImageView(imageView);
    }
    this->layerImageViews.clear();

    // Mip image views
    for (auto& imageView : this->mipImageViews)
    {
        this->device->getVkDevice().destroyImageView(imageView);
    }
    this->mipImageViews.clear();

    // Image
    this->device->getVkDevice().destroyImage(this->image);
    vmaFreeMemory(*this->vma, this->imageMemory);
}

vk::Format Texture::chooseSupportedFormat(
    PhysicalDevice& physicalDevice,
    const std::vector<vk::Format>& formats,
    const vk::ImageTiling& tiling,
    const vk::FormatFeatureFlagBits& featureFlags)
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

vk::Format Texture::getDepthBufferFormat(PhysicalDevice& physicalDevice)
{
    return Texture::chooseSupportedFormat(
        physicalDevice,
        {
            // Atleast one of these should be available...
            vk::Format::eD32Sfloat,
            vk::Format::eD24UnormS8Uint
        },
        vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment // Make sure the format supports the depth stencil attachment bit
    );
}

vk::Image Texture::createImage(
    VmaAllocator& vma,
    ImageCreateData&& imageData)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    // - Create Image - 
    // Image Createion Info
    vk::ImageCreateInfo imageCreateInfo;
    imageCreateInfo.setImageType(vk::ImageType::e2D);
    imageCreateInfo.setExtent(
        vk::Extent3D(
            imageData.width,
            imageData.height,
            1
        )
    );
    imageCreateInfo.setMipLevels(imageData.mipLevels);                        // number of mipmap levels 
    imageCreateInfo.setArrayLayers(imageData.arrayLayers);                        // number of Levels in image array
    imageCreateInfo.setFormat(imageData.format);                   // Format of the image
    imageCreateInfo.setTiling(imageData.tiling);                   // How image data should be "tiled" (arranged for optimal reading speed)
    imageCreateInfo.setInitialLayout(vk::ImageLayout::eUndefined);// Layout of image data on creation
    imageCreateInfo.setUsage(imageData.useFlags);                 // Flags defining how the image will be used
    imageCreateInfo.setSamples(vk::SampleCountFlagBits::e1);    // Number of samples to be used for multi-sampling (1 since we dont use multisampling)
    imageCreateInfo.setSharingMode(vk::SharingMode::eExclusive);// wheter the image will be shared between queues (we will not share images between queues...)

    VmaAllocationCreateInfo vmaAllocCreateInfo{};
    vmaAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;

    vk::Image image;

    // Create the Image
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
	const vk::Image& image,
	const vk::Format& format,
	const vk::ImageAspectFlags& aspectFlags,
    const vk::ImageViewType& imageViewType,
    const uint32_t& arrayLayers,
    const uint32_t& arrayLayerSlice,
    const bool& useEntireArray,
    const uint32_t& mipLevels,
    const uint32_t& mipSlice
)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    // Creating a ImageView is similar to how we have the Physical Device and from that create a Logical Device...
    vk::ImageViewCreateInfo viewCreateInfo;

    viewCreateInfo.setImage(image);                           // Image to create view for
    viewCreateInfo.setViewType(imageViewType);                           // Type of Image (1D, 2D, 3D, Cube, etc)
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
    viewCreateInfo.subresourceRange.baseMipLevel = mipSlice;                // Which part of the image to view start view from, (a Image can have multiple Mip and Array Layers)...
    viewCreateInfo.subresourceRange.levelCount = 1;                // How many MipMap levels to view, we only view 1 and that will be the "0" referred to by baseMipLevel
    viewCreateInfo.subresourceRange.baseArrayLayer = arrayLayerSlice;                // Which BaseArrayLayer to start from, we pick the first: 0
    viewCreateInfo.subresourceRange.layerCount = !useEntireArray ? 1 : arrayLayers;                // How many layers to check from the baseArrayLayer.

    // Create image view and Return it
    return device.getVkDevice().createImageView(viewCreateInfo);
}

void Texture::transitionImageLayout(
    Device& device, 
    const vk::Queue& queue,
    const vk::CommandPool& commandPool, 
    const vk::Image& image,
    const vk::ImageLayout& oldLayout,
    const vk::ImageLayout& newLayout)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //: NOLINT
#endif
    // Create Buffer
    vk::CommandBuffer commandBuffer = 
        CommandBuffer::beginCommandBuffer(
            device.getVkDevice(), 
            commandPool
        );

    // MemoryBarrier allows us to transition between two layouts!
    vk::ImageMemoryBarrier2 imageMemoryBarrier{};
    imageMemoryBarrier.setOldLayout(oldLayout); // Layout to transition from
    imageMemoryBarrier.setNewLayout(newLayout); // Layout to transition from
    imageMemoryBarrier.setSrcQueueFamilyIndex(
        VK_QUEUE_FAMILY_IGNORED); // Queue Family to transition from (used for queue family ownership transfer)
    imageMemoryBarrier.setDstQueueFamilyIndex(
        VK_QUEUE_FAMILY_IGNORED); // Queue Family to transition to  (used for queue family ownership transfer)
    imageMemoryBarrier.setImage(image);

    imageMemoryBarrier.subresourceRange.setAspectMask(
        vk::ImageAspectFlagBits::eColor); // Aspect of the image being altered
    imageMemoryBarrier.subresourceRange.setBaseMipLevel(
        uint32_t(0)); // First mip level to start alterations on
    imageMemoryBarrier.subresourceRange.setLevelCount(
        uint32_t(1)); // Number of mipmap levels to alter starting from baseMipLevel
    imageMemoryBarrier.subresourceRange.setBaseArrayLayer(
        uint32_t(0)); // First layers to start alterations on
    imageMemoryBarrier.subresourceRange.setLayerCount(
        uint32_t(1)); // Number of layers to alter starting from baseArrayLayer

    // If transitioning from 'new image' to 'image ready' to 'recieve data'
    if (oldLayout == vk::ImageLayout::eUndefined &&
        newLayout == vk::ImageLayout::eTransferDstOptimal) 
    {
        // Defining that before ANY TransferWrite happens (no matter what state
        // it is in),
        /// a transition between oldLayout to newLayout MUST happen!
        /// i.e. A layout transition MUST happen before we attempt to do a
        //Transfer Write (VK_ACCESS_TRANSFER_WRITE_BIT)
        imageMemoryBarrier.setSrcAccessMask(
            vk::AccessFlagBits2::eNone); // srcState 0 means any State TODO::
        // Does none work here?
        imageMemoryBarrier.setDstAccessMask(
            vk::AccessFlagBits2::eTransferWrite); // The dstState cant execute
        // before srcState finishes!
// transition must happens after srcAccessMask
// transition must happens before dstAccessMask

        imageMemoryBarrier.setSrcStageMask(
            vk::PipelineStageFlagBits2::
            eTopOfPipe); // Transition must happen  after srcStage: Any
        // stage from (at?) the top of the pipeline
        imageMemoryBarrier.setDstStageMask(
            vk::PipelineStageFlagBits2::
            eTransfer); // Transition must happen before dstStage: The
        // TransferStage of the Pipeline (where it tried to
        // do a TransferWrite!)

    } // If transitioning from Transfer Destination to Shader Readable
    else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
        newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) 
    {
        imageMemoryBarrier.setSrcAccessMask(
            vk::AccessFlagBits2::
            eTransferWrite); // The srcState must be finished with writing
        // before layout transition can happen!
        imageMemoryBarrier.setDstAccessMask(
            vk::AccessFlagBits2::eShaderRead); // The dstState transition
        // must've happen before Shader
        // will read the data.

        imageMemoryBarrier.setSrcStageMask(
            vk::PipelineStageFlagBits2::
            eTransfer); // Transition must happen  after srcStage:
        // Transfered the data to the ... (??)

        imageMemoryBarrier.setDstStageMask(
            vk::PipelineStageFlagBits2::
            eFragmentShader); // Transition must happen before dstStage:
        // Transfer the data to the Fragment Shader
        // Stage
    }

    // Generic Pipeline Barrier is used for all Barriers, thus some arguments
    // will not be used
    vk::DependencyInfo dependencyInfo;
    dependencyInfo.setDependencyFlags(
        vk::DependencyFlagBits::eByRegion); // TODO :: is this the same as
    // setting flagBits to 0? ...
    dependencyInfo.setBufferMemoryBarrierCount(uint32_t(0));
    dependencyInfo.setPBufferMemoryBarriers(nullptr);
    dependencyInfo.setImageMemoryBarrierCount(uint32_t(1));
    dependencyInfo.setImageMemoryBarriers(imageMemoryBarrier);
    dependencyInfo.setMemoryBarrierCount(uint32_t(0));
    dependencyInfo.setPMemoryBarriers(nullptr);

    commandBuffer.pipelineBarrier2(dependencyInfo);

    CommandBuffer::endAndSubmitCommandBuffer(
        device.getVkDevice(), 
        commandPool, 
        queue, 
        commandBuffer
    );
}