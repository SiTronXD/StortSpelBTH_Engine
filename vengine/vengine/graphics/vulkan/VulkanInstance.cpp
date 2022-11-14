#include "pch.h"
#include "../../application/Window.hpp"
#include "../../dev/Log.hpp"

bool VulkanInstance::checkInstanceExtensionSupport(
    std::vector<const char*>* checkExtensions)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    // Fetch the instance extensions properties...
    std::vector<vk::ExtensionProperties> extensions =
        vk::enumerateInstanceExtensionProperties();

    //Check if the given extensions is in the list of valid extensions
    for (auto& checkExtension : *checkExtensions)
    {
        bool hasExtension = false;
        for (auto& extension : extensions)
        {
            if (strcmp(checkExtension, extension.extensionName) == 0)
            {
                hasExtension = true;
                break;
            }
        }
        if (!hasExtension)
        {
            return false;
        }
    }

    return true;
}

VulkanInstance::VulkanInstance()
{
}

VulkanInstance::~VulkanInstance()
{
}

void VulkanInstance::createInstance(Window& window)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    // Check if in debug mode and if we can use the Vulkan Validation Layers...
    if (isValidationLayersEnabled() && !Validation::checkValidationLayerSupport())
    {
        Log::error("Tried to use a non-available validation layer...");
    }

#ifndef NDEBUG
    if (!isValidationLayersEnabled())
    {
        std::cout << "Warning! in Debug mode, but programmatic warnings are disabled!\n";
    }
#endif

    // VkApplication info is used by VkInstanceCreateInfo, its info about the application we're making
    // This is used by developers, for things like debugging ...
    vk::ApplicationInfo appInfo = {};
    appInfo.setPApplicationName(window.getTitleName().c_str());               // Custom Name of the application
    appInfo.setApplicationVersion(VK_MAKE_VERSION(1, 0, 0));      // Description of which version this is of our program
    appInfo.setPEngineName("Vengine");                   // Name of the used Engine
    appInfo.setEngineVersion(VK_MAKE_VERSION(1, 0, 0));           // Description of which version this is of the Engine    
    appInfo.setApiVersion(VK_API_VERSION_1_3);                  // Version of the Vulkan API that we are using!

    //Creation information for a vk instance (vulkan instance)
    vk::InstanceCreateInfo createInfo({}, &appInfo);

    //Create vector to hold instance extensions.
    auto instanceExtensions = std::vector<const char*>();

    // Get SDL extensions from window
    std::vector<const char*> windowExtensions;
    window.getVulkanExtensions(windowExtensions);

    // Add SDL extensions to vector of extensions    
    for (size_t i = 0; i < windowExtensions.size(); i++)
    {
        instanceExtensions.push_back(windowExtensions[i]);    //One of these extension should be VK_KHR_surface, this is provided by SDL!
    }

    //Check if any of the instance extensions is not supported...
    if (!this->checkInstanceExtensionSupport(&instanceExtensions))
    {
        Log::error("vk::Instance does not support at least one of the required extension!");
    }

    if (isValidationLayersEnabled())
    {
        // When we use Validation Layers we want add a extension in order to get Message Callback...
        instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

#ifdef DEBUG
    instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif 
    //instanceExtensions.push_back(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME); (!!)

    instanceExtensions.insert(instanceExtensions.end(), extraInstanceExtensions.begin(), extraInstanceExtensions.end());

    // We add the Extensions to the createInfo,
    // Note: we use static_cast to convert the size_t (returned from size) to uint32_t,
    // this is to avoid problems with different implementations...
    createInfo.setEnabledExtensionCount(static_cast<uint32_t>(instanceExtensions.size()));
    createInfo.setPpEnabledExtensionNames(instanceExtensions.data());

    if (isValidationLayersEnabled())
    {
        // The validation layers are defined in my VulkanValidation.h file.
        createInfo.setEnabledLayerCount(static_cast<uint32_t>(validationLayers.size()));
        createInfo.setPpEnabledLayerNames(validationLayers.data());

    }
    else
    {
        createInfo.setEnabledLayerCount(uint32_t(0));       // Set to zero, right now we do not want any validation layers        
        createInfo.setPpEnabledLayerNames(nullptr);         // since we don't use validation layer right now
    }

    vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

    vk::ValidationFeaturesEXT validationFeatures{};
    std::vector<vk::ValidationFeatureEnableEXT> disabledValidationFeatures{};
    

    // Enable Debugging in createInstance function (special Case...)
    if (isValidationLayersEnabled())
    {
        VulkanDbg::populateDebugMessengerCreateInfo(debugCreateInfo);

        std::vector<vk::ExtensionProperties> layerProperties = vk::enumerateInstanceExtensionProperties();

        validationFeatures.setDisabledValidationFeatureCount(0);
        validationFeatures.setPDisabledValidationFeatures(nullptr);
        validationFeatures.setEnabledValidationFeatureCount(static_cast<uint32_t>(enabledValidationFeatures.size()));
        validationFeatures.setPEnabledValidationFeatures(enabledValidationFeatures.data());

        debugCreateInfo.setPNext(&validationFeatures);
        createInfo.setPNext((vk::DebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo);
    }
    else
    {
        createInfo.setPNext(nullptr);
    }

    //Create the instance!
    this->instance = vk::createInstance(createInfo);
}

void VulkanInstance::destroy(vk::SurfaceKHR& surface)
{
    this->instance.destroy(surface);
}

void VulkanInstance::cleanup()
{
    this->instance.destroy();   // <- I think this gets taken care of by the destructor... maybe? 
}