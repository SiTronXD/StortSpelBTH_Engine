#include "PhysicalDevice.h"
#include "Utilities.h"
#include "Log.h"

void PhysicalDevice::getSwapchainDetails(
    const vk::PhysicalDevice& physDevice,
    vk::SurfaceKHR& surface,
    SwapChainDetails& outputDetails)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    outputDetails = {};

    // Surface capabilities
    vk::PhysicalDeviceSurfaceInfo2KHR physicalDeviceSurfaceInfo{};
    physicalDeviceSurfaceInfo.setSurface(surface);
    outputDetails.surfaceCapabilities =
        physDevice.getSurfaceCapabilities2KHR(physicalDeviceSurfaceInfo);

    // Formats
    outputDetails.Format =
        physDevice.getSurfaceFormats2KHR(physicalDeviceSurfaceInfo);

    // Presentation modes
    outputDetails.presentationMode =
        physDevice.getSurfacePresentModesKHR(surface);
}

bool PhysicalDevice::checkPhysicalDeviceSuitability(
    const vk::PhysicalDevice& physDevice, 
    vk::SurfaceKHR& surface,
    SwapChainDetails& outputSwapChainDetails)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    
    // Information about the device itself (Id, name, type, vendor, etc...)
    vk::PhysicalDeviceProperties deviceProperties = physDevice.getProperties(); //TODO: Unused, use or remove

    ///Check if the Device has the required Features... 
    bool deviceHasSupportedFeatures = false;
    vk::PhysicalDeviceFeatures deviceFeatures = physDevice.getFeatures();
    if (deviceFeatures.samplerAnisotropy == VK_TRUE) //TODO: Extract into a vector similar to what we've done with extensions
    {
        deviceHasSupportedFeatures = true;
    }
    
    QueueFamilyIndices indices = this->getQueueFamilies(physDevice, surface);

    bool extensions_supported = this->checkDeviceExtensionSupport(physDevice);

    this->getSwapchainDetails(
        physDevice,
        surface,
        outputSwapChainDetails
    );

    /// Once we have the QueueFamily Indices we can return whatever our "isValid" function concludes...
    return indices.isValid() &&
        extensions_supported &&
        outputSwapChainDetails.isValid() &&
        deviceHasSupportedFeatures;
}

bool PhysicalDevice::checkDeviceExtensionSupport(
    const vk::PhysicalDevice& physDevice)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    // Fetch the Physical Device Extension properties...
    std::vector<vk::ExtensionProperties> deviceExtensionProperties = 
        physDevice.enumerateDeviceExtensionProperties();

    // Return false if Physical Device does not support any Extensions
    if (deviceExtensionProperties.empty()) 
    {
        return false;
    }

    // Make sure all requested extensions exists on the Physical Device
    for (const auto& requested_extensionProperty : deviceExtensions) {

        bool required_extension_exists = false;
        for (const auto& device_extensionProperty : deviceExtensionProperties)
        {
            if (strcmp(requested_extensionProperty, device_extensionProperty.extensionName) == 0) 
            {
                required_extension_exists = true;
                break;
            }
        }
        if (!required_extension_exists) 
        {
            return false;
        }
    }

    return true;
}

QueueFamilyIndices PhysicalDevice::getQueueFamilies(
    const vk::PhysicalDevice& physDevice,
    vk::SurfaceKHR& surface)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    QueueFamilyIndices indices;

    // Get Queue Families
    std::vector<vk::QueueFamilyProperties2> queueFamilies = 
        physDevice.getQueueFamilyProperties2();

    // Look for queue family indices
    int32_t queueFamilyIndex = 0;
    for (const auto& queueFamily : queueFamilies) 
    {
        // Check for graphics queue index
        if (queueFamily.queueFamilyProperties.queueCount > 0 && 
            (queueFamily.queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics)) 
        {
            indices.graphicsFamily = queueFamilyIndex; 
        }

        // Check for present queue index
        vk::Bool32 surfaceHasSupport = VK_FALSE;
        surfaceHasSupport = 
            physDevice.getSurfaceSupportKHR(queueFamilyIndex, surface);
        if (queueFamily.queueFamilyProperties.queueCount > 0 && 
            (surfaceHasSupport != 0U)) 
        {
            indices.presentationFamily = queueFamilyIndex;
        }

        // Check if graphics queue and presentation queue have been found
        if (indices.isValid()) 
        {
            break;
        }

        queueFamilyIndex++;
    }

    return indices;
}

PhysicalDevice::PhysicalDevice()
{
}

PhysicalDevice::~PhysicalDevice()
{
}

void PhysicalDevice::pickPhysicalDevice(
    vk::Instance& instance,
    vk::SurfaceKHR& surface,
    QueueFamilyIndices& outputQueueFamilies,
    SwapChainDetails& outputSwapChainDetails)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    // We need to pick which of the systems physical device to be used; Integrated GPU, One of Multiple GPU, External GPU... etc
    auto allPhysicalDevices = instance.enumeratePhysicalDevices();

    // Check if any devices where available, if none then we dont have support for vulkan...
    if (allPhysicalDevices.empty())
    {
        Log::error("Can't find GPUs that support Vulkan instances...");
    }

    // Loop through all possible devices and pick the first suitable device!    
    for (const auto& physDevice : allPhysicalDevices)
    {
        if (this->checkPhysicalDeviceSuitability(
            physDevice, 
            surface, 
            outputSwapChainDetails))
        {
            this->physicalDevice = physDevice;
            break;
        }
    }
    outputQueueFamilies = this->getQueueFamilies(this->physicalDevice, surface);
}
