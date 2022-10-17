#include <set>

#include "Device.hpp"
#include "VulkanDbg.hpp"
#include "VulkanInstance.hpp"
#include "QueueFamilies.hpp"

Device::Device()
{
}

Device::~Device()
{
}

void Device::createDevice(
    VulkanInstance& instance,
    PhysicalDevice& physicalDevice, 
    QueueFamilies& outputQueueFamilies,
    
    // TODO: remove this
    vk::DispatchLoaderDynamic& outputDynamicDispatch)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    int32_t graphicsQueueIndex = outputQueueFamilies.getGraphicsIndex();
    int32_t presentQueueIndex = outputQueueFamilies.getPresentIndex();

    // Using a vector to store all the queueCreateInfo structs for each QueueFamily...
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

    // Using a set which will store all our indices;
    // When using a set we can represent 1 index for each QueueFamily without risk of adding the same queueFamily index multiple times!
    std::set<int32_t> queueFamilyIndices
    {
        graphicsQueueIndex,
        presentQueueIndex    // This could be the same Queue Family as the GraphicsQueue Family! thus, we use a set!
    };

    float priority = 1.F;
    // Queues the Logical device needs to create, and info to do so; Priority 1 is highest, 0 is lowest...
    // - Note; if the graphicsFamily and presentationFamily is the same, this loop will only do 1 iteration, else 2...
    for (std::size_t i = 0; i < queueFamilyIndices.size(); i++)
    {

        vk::DeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.setQueueFamilyIndex(graphicsQueueIndex);         // The index of the family to create Graphics queue from
        queueCreateInfo.setQueueCount(uint32_t(1));

        // We can create Multiple Queues, and thus this is how we decide what Queue to prioritize...        
        queueCreateInfo.setPQueuePriorities(&priority); // Since we only have One priority, we will use a pointer to the priority value...

        // Add each of the *Unique* queue Family Indices createQueueInfo instances to the queueCreateInfos vector!
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // Information to create a logical device (Logical devices are most commonly referred to as 'device', rather than 'logical device'...)
    vk::DeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.setQueueCreateInfoCount(static_cast<uint32_t>(queueCreateInfos.size())); // Number of queueCreateInfos!
    deviceCreateInfo.setPQueueCreateInfos(queueCreateInfos.data());     // List of queue create infos so that the device can require Queues...

    // The extensions we want to use are stored in our Global deviceCreateInfo array!
    deviceCreateInfo.setEnabledExtensionCount(static_cast<uint32_t>(deviceExtensions.size()));           // We dont have any logical Device Extensions (not the same as instance extensions...)
    deviceCreateInfo.setPpEnabledExtensionNames(deviceExtensions.data());// since we dont have any... use nullptr! (List of Enabled Logical device extensions

    //Physical Device features the logical Device will be using...
    vk::PhysicalDeviceFeatures2 deviceFeatures{};
    deviceFeatures.features.setSamplerAnisotropy(VK_TRUE);             // Enables the Anisotropy Feature, now a Sampler made for this Device can use Anisotropy!                 

    // Get extension Features    
    vk::PhysicalDeviceSynchronization2Features physicalDeviceSyncFeatures{};
    vk::PhysicalDeviceDynamicRenderingFeatures physicalDeviceDynamicRenderingFeatures{ VK_TRUE };
    physicalDeviceSyncFeatures.setPNext(&physicalDeviceDynamicRenderingFeatures);
    deviceFeatures.setPNext(&physicalDeviceSyncFeatures);
    physicalDeviceSyncFeatures.setSynchronization2(VK_TRUE);

    deviceCreateInfo.setPNext(&deviceFeatures);


    // Create the logical device for the given Physical Device
    this->device = physicalDevice.getVkPhysicalDevice().createDevice(deviceCreateInfo);

    // Setup Dynamic Dispatch, in order to use device extensions        
    outputDynamicDispatch = vk::DispatchLoaderDynamic(
        instance.getVkInstance(), 
        vkGetInstanceProcAddr, 
        this->device
    );

    // Queues are Created at the same time as the device...
    // So we want handle to queues:
    vk::DeviceQueueInfo2 graphicsQueueInfo;
    graphicsQueueInfo.setQueueFamilyIndex(static_cast<uint32_t>(graphicsQueueIndex));
    graphicsQueueInfo.setQueueIndex(uint32_t(0));
    outputQueueFamilies.setGraphicsQueue(this->device.getQueue2(graphicsQueueInfo));

    // Add another handle to let the Logical Device handle PresentationQueue... (??)
    vk::DeviceQueueInfo2 presentationQueueInfo;
    presentationQueueInfo.setQueueFamilyIndex(static_cast<uint32_t>(presentQueueIndex));
    presentationQueueInfo.setQueueIndex(uint32_t(0));        // Will be positioned at the Queue index 0 for This particular family... (??)
    outputQueueFamilies.setPresentQueue(this->device.getQueue2(presentationQueueInfo));
}

void Device::destroyBuffer(vk::Buffer& buffer)
{
    this->device.destroyBuffer(buffer);
}

void Device::waitIdle()
{
    this->device.waitIdle();
}

void Device::cleanup()
{
    this->device.destroy();
}
