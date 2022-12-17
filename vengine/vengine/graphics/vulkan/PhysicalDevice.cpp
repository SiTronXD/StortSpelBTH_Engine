#include "pch.h"
#include "Swapchain.hpp"
#include "../../dev/Log.hpp"

bool PhysicalDevice::checkPhysicalDeviceSuitability(
    vk::PhysicalDevice& physDevice, 
    vk::SurfaceKHR& surface)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    
    ///Check if the Device has the required Features... 
    bool deviceHasSupportedFeatures = false;
    vk::PhysicalDeviceFeatures deviceFeatures = physDevice.getFeatures();
    if (deviceFeatures.samplerAnisotropy == VK_TRUE) //TODO: Extract into a vector similar to what we've done with extensions
    {
        deviceHasSupportedFeatures = true;
    }
    
    QueueFamilyIndices indices = 
        this->getQueueFamilies(physDevice, surface);

    bool extensions_supported = this->checkDeviceExtensionSupport(physDevice);

    SwapchainDetails swapchainDetails{};
    Swapchain::getDetails(
        physDevice,
        surface,
        swapchainDetails
    );

    /// Once we have the QueueFamily Indices we can return whatever our "isValid" function concludes...
    return indices.isValid() &&
        extensions_supported &&
        swapchainDetails.isValid() &&
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
    for (const auto& requested_extensionProperty : deviceExtensions) 
    {

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

        // Check for compute queue index
        if (queueFamily.queueFamilyProperties.queueCount > 0 &&
            (queueFamily.queueFamilyProperties.queueFlags & vk::QueueFlagBits::eCompute))
        {
            indices.computeFamily = queueFamilyIndex;
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
    VulkanInstance& instance,
    vk::SurfaceKHR& surface,
    QueueFamilyIndices& outputQueueFamilies)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    // We need to pick which of the systems physical device to be used; Integrated GPU, One of Multiple GPU, External GPU... etc
    auto allPhysicalDevices = instance.getVkInstance().enumeratePhysicalDevices();

    // Check if any devices where available, if none then we dont have support for vulkan...
    if (allPhysicalDevices.empty())
    {
        Log::error("Can't find GPUs that support Vulkan instances...");
    }

    // Loop through all possible devices and pick the first suitable device!
    bool foundPhysicalDevice = false;
    for (auto& physDevice : allPhysicalDevices)
    {
        if (this->checkPhysicalDeviceSuitability(
            physDevice,
            surface))
        {
            this->physicalDevice = physDevice;
            foundPhysicalDevice = true;
            break;
        }
    }

    // Could not find a proper physical device
    if (!foundPhysicalDevice)
    {
        Log::error("Could not find a properly supported GPU. Make sure the latest drivers are installed.");

        return;
    }
    // Physical device found
    else
    {
        vk::PhysicalDeviceProperties deviceProperties = this->physicalDevice.getProperties();

        std::string gpuName = deviceProperties.deviceName;
        std::string gpuApiVersion =
            std::to_string(VK_API_VERSION_MAJOR(deviceProperties.apiVersion)) + "." +
            std::to_string(VK_API_VERSION_MINOR(deviceProperties.apiVersion)) + "." + 
            std::to_string(VK_API_VERSION_PATCH(deviceProperties.apiVersion));

        Log::write("GPU name: " + gpuName);
        Log::write("GPU API version: " + gpuApiVersion);
    }

    outputQueueFamilies = this->getQueueFamilies(this->physicalDevice, surface);
}
