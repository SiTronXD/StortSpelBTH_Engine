#pragma once
#include <vulkan/vulkan.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <array>

#include "Configurator.h"

/// Defines the Validation layers to be used (??)
//constexpr std::vector<const char*>   validationLayers = {
constexpr std::array<const char*, 1>   validationLayers = {
        "VK_LAYER_KHRONOS_validation"   /// This means we use All standard validation layers!
        
};

/// Returns whether programmatic validation should be used
////__attribute__((unused))
[[maybe_unused]]  // To remove annoying warning... this function is used in another translation unit...
static bool isValidationLayersEnabled(){    
#ifdef  NDEBUG
    return false;
#else
    return vengine_helper::config::DEF<bool>(USE_BUILTIN_VALIDATION);
#endif
    
}

namespace Validation {
    bool checkValidationLayerSupport();
}

VkResult CreateDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger
        );

void DestroyDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks* pAllocator
        );


////__attribute__((unused))
[[maybe_unused]]  // To remove annoying warning... this function is used in another translation unit...
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                    [[maybe_unused]] void* pUserData){ ///Added [[maybe_unused]] to get rid of some warnings
/// messageSeverity:
/// Severity of the Message, one of the following:
/// - VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT   : Diagnostic message
/// - VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT      : Informational message, like creation of resource
/// - VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT   : Describes a weird behavior that's likely a bug
/// - VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT     : Invalid behavior that cause crashes

/// messageType:
/// - VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT       : Something Unrelated to specification or performance has happened
/// - VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT    : Something has happened that indication a violation or mistake
/// - VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT   : Potential non-optimal use of vulkan

/// pCallbackData (instance of VkDebugUtilsMessengerCallbackDataEXT struct):
/// - pMessage       : the message to display...
/// - pObjects    : array of vulkan objects related to the message
/// - objectCount   : number of object in the aforementioned array

/// pUserData:
/// Is a variable that we as the author of the callback function can use...

    static std::map<std::string, uint32_t> messages;
    
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {    
        std::string formattedMessage = std::string(pCallbackData->pMessage);         

        auto message = messages.find(formattedMessage);
        if ((message == messages.end()))
        {            

            switch (messageType) {
                case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
                    std::cerr << "Validation Layer:GENR: " << formattedMessage << "\n\n";
                    break;
                case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
                    std::cerr << "Validation Layer:VALI: " << formattedMessage << "\n\n";
                    break;
                case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
                    std::cerr << "Validation Layer:PERF: " << formattedMessage << "\n\n";
                    break;

                default:

                    break;            
            }

           messages.insert({formattedMessage, 0});
        }
        else
        {
            message->second++;
        }
    }
    
    return VK_FALSE; /// Should generally only return VK_FALSE...
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_Callback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT type,
                                                     uint64_t object, size_t location, int32_t message_code,
                                                     const char *layer_prefix, const char *message, void *user_data)
{
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
	{
		printf("Validation Layer: Error: {}: {}", layer_prefix, message);
	}
	else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
	{
		printf("Validation Layer: Warning: {}: {}", layer_prefix, message);
	}
	else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
	{
		printf("Validation Layer: Performance warning: {}: {}", layer_prefix, message);
	}
	else
	{
		printf("Validation Layer: Information: {}: {}", layer_prefix, message);
	}
	return VK_FALSE;
}