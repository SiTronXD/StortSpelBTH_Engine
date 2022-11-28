#include "pch.h"

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
    int32_t computeQueueIndex = outputQueueFamilies.getComputeIndex();

    // Using a vector to store all the queueCreateInfo structs for each QueueFamily...
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

    // Using a set which will store all our indices;
    // When using a set we can represent 1 index for each QueueFamily without risk of adding the same queueFamily index multiple times!
    std::set<int32_t> queueFamilyIndices
    {
        graphicsQueueIndex,
        presentQueueIndex,  // This could be the same queue family as the graphics queue family. Thus, we use a set.
        computeQueueIndex   // This could be the same queue family as the graphics queue family. Thus, we use a set.
    };

    // Create one vk::DeviceQueueCreateInfo per unique queue index
    float priority = 1.F;
    for (auto& queueIndex : queueFamilyIndices)
    {
        vk::DeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.setQueueFamilyIndex(queueIndex);
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

    // Physical device features the logical device will be using...
    vk::PhysicalDeviceFeatures2 deviceFeatures{};
    deviceFeatures.features.setSamplerAnisotropy(VK_TRUE);             // Enables the Anisotropy Feature, now a Sampler made for this Device can use Anisotropy!                 
#if defined(_CONSOLE) // Debug/Release, but not distribution
    deviceFeatures.features.setFillModeNonSolid(VK_TRUE);
#endif

    // Get extension features    
    vk::PhysicalDeviceSynchronization2Features physicalDeviceSyncFeatures{};
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

    // Get graphics queue handle
    vk::DeviceQueueInfo2 graphicsQueueInfo;
    graphicsQueueInfo.setQueueFamilyIndex(static_cast<uint32_t>(graphicsQueueIndex));
    graphicsQueueInfo.setQueueIndex(uint32_t(0));
    outputQueueFamilies.setGraphicsQueue(this->device.getQueue2(graphicsQueueInfo));

    // Get presentation queue handle
    vk::DeviceQueueInfo2 presentationQueueInfo;
    presentationQueueInfo.setQueueFamilyIndex(static_cast<uint32_t>(presentQueueIndex));
    presentationQueueInfo.setQueueIndex(uint32_t(0));
    outputQueueFamilies.setPresentQueue(this->device.getQueue2(presentationQueueInfo));

    // Get compute queue handle
    vk::DeviceQueueInfo2 computeQueueInfo;
    computeQueueInfo.setQueueFamilyIndex(static_cast<uint32_t>(computeQueueIndex));
    computeQueueInfo.setQueueIndex(uint32_t(0));
    outputQueueFamilies.setComputeQueue(this->device.getQueue2(computeQueueInfo));
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
