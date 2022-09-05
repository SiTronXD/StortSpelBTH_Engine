#include "VulkanValidation.h"
#include "tracy/Tracy.hpp"

///Checks all available validation layers, returns false if requested layer not found
bool Validation::checkValidationLayerSupport() {
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    uint32_t layerCount = 0;

    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableValidationLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableValidationLayers.data());

    for (const auto &requestedValidationLayer : validationLayers) {
        bool found = false;
        for (auto &availableLayer : availableValidationLayers) {
            if (strcmp(requestedValidationLayer,availableLayer.layerName) == 0) {
                found = true;
                break;
            }
        }
        if (!found){
            return false;
        }
    }
    return true;
}


VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                      const VkAllocationCallbacks *pAllocator,
                                      VkDebugUtilsMessengerEXT *pDebugMessenger) {

    ///This function is used to create the VkDebugUtilsMessengerEXT object,
    ///since it's a Extension we have to figure out its address (which is done with vkGetInstanceProcAddr func...)

    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo,pAllocator,pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks *pAllocator) {

    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

    if (func != nullptr) {
        func(instance,debugMessenger,pAllocator);
    }

}

