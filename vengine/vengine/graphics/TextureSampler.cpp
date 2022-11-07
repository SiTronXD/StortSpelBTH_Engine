#include "pch.h"
#include "TextureSampler.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/VulkanDbg.hpp"
#include "../resource_management/Configurator.hpp"

using namespace vengine_helper::config;

void TextureSampler::createSampler(
    Device& device,
    const TextureSamplerSettings& settings)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    this->device = &device;
    
    // Max LOD
    float maxLod =
        !settings.unnormalizedCoordinates ?
        VK_LOD_CLAMP_NONE :
        0.0f;

    // Mipmap mode
    vk::SamplerMipmapMode mipmapMode =
        settings.filterMode == vk::Filter::eLinear ?
        vk::SamplerMipmapMode::eLinear :
        vk::SamplerMipmapMode::eNearest;

    // Address mode
    vk::SamplerAddressMode addressMode =
        !settings.unnormalizedCoordinates ?
        vk::SamplerAddressMode::eRepeat :
        vk::SamplerAddressMode::eClampToEdge;

    // Anisotropy enabled
    vk::Bool32 anisotropyEnabled =
        !settings.unnormalizedCoordinates ?
        VK_TRUE :
        VK_FALSE;

    // Sampler creation info
    vk::SamplerCreateInfo samplerCreateInfo;
    samplerCreateInfo.setMagFilter(settings.filterMode);                     // How the sampler will sample from a texture when it's getting closer
    samplerCreateInfo.setMinFilter(settings.filterMode);                     // How the sampler will sample from a texture when it's getting further away
    samplerCreateInfo.setAddressModeU(addressMode);      // How the texture will be Wrapped in U (x) direction
    samplerCreateInfo.setAddressModeV(addressMode);      // How the texture will be Wrapped in V (y) direction
    samplerCreateInfo.setAddressModeW(addressMode);      // How the texture will be Wrapped in W (z) direction
    samplerCreateInfo.setBorderColor(vk::BorderColor::eIntOpaqueBlack);      // Color of what is around the texture (in case of Repeat, it wont be used)
    samplerCreateInfo.setUnnormalizedCoordinates(settings.unnormalizedCoordinates);                  // Normalized [0, 1) or unnormalized [0, width) coordinates
    samplerCreateInfo.setMipmapMode(mipmapMode);         // How the mipmap mode will switch between the mipmap images (interpolate between images), (we dont use it, but we set it up)
    samplerCreateInfo.setMipLodBias(0.F);                                    // Level of detail bias for mip level...
    samplerCreateInfo.setMinLod(0.F);                                        // Minimum level of Detail to pick mip level
    samplerCreateInfo.setMaxLod(maxLod);                          // Maxiumum level of Detail to pick mip level
    samplerCreateInfo.setAnisotropyEnable(anisotropyEnabled);                           // Enable Anisotropy; take into account the angle of a surface is being viewed from and decide details based on that (??)

    samplerCreateInfo.setMaxAnisotropy(DEF<float>(SAMPL_MAX_ANISOSTROPY));   // Level of Anisotropy; 16 is a common option in the settings for alot of Games 

    // Create sampler
    this->textureSampler = this->device->getVkDevice().createSampler(
        samplerCreateInfo);
    VulkanDbg::registerVkObjectDbgInfo("Texture Sampler", vk::ObjectType::eSampler, reinterpret_cast<uint64_t>(vk::Sampler::CType(this->textureSampler)));
}

void TextureSampler::cleanup()
{
	this->device->getVkDevice().destroySampler(this->textureSampler);
}

void TextureSampler::settingsToString(
    const TextureSamplerSettings& samplerSettings,
    std::string& outputString)
{
    outputString = "";
    outputString += std::to_string((int)samplerSettings.filterMode) + ";";
    outputString += std::to_string((int)samplerSettings.unnormalizedCoordinates) + ";";
}