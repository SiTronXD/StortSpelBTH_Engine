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

    // Sampler creation info
    vk::SamplerCreateInfo samplerCreateInfo;
    samplerCreateInfo.setMagFilter(settings.filterMode);                     // How the sampler will sample from a texture when it's getting closer
    samplerCreateInfo.setMinFilter(settings.filterMode);                     // How the sampler will sample from a texture when it's getting further away
    samplerCreateInfo.setAddressModeU(vk::SamplerAddressMode::eRepeat);      // How the texture will be Wrapped in U (x) direction
    samplerCreateInfo.setAddressModeV(vk::SamplerAddressMode::eRepeat);      // How the texture will be Wrapped in V (y) direction
    samplerCreateInfo.setAddressModeW(vk::SamplerAddressMode::eRepeat);      // How the texture will be Wrapped in W (z) direction
    samplerCreateInfo.setBorderColor(vk::BorderColor::eIntOpaqueBlack);      // Color of what is around the texture (in case of Repeat, it wont be used)
    samplerCreateInfo.setUnnormalizedCoordinates(VK_FALSE);                  // We want to used Normalised Coordinates (between 0 and 1), so unnormalized coordinates must be false... 
    samplerCreateInfo.setMipmapMode(vk::SamplerMipmapMode::eLinear);         // How the mipmap mode will switch between the mipmap images (interpolate between images), (we dont use it, but we set it up)
    samplerCreateInfo.setMipLodBias(0.F);                                    // Level of detail bias for mip level...
    samplerCreateInfo.setMinLod(0.F);                                        // Minimum level of Detail to pick mip level
    samplerCreateInfo.setMaxLod(VK_LOD_CLAMP_NONE);                          // Maxiumum level of Detail to pick mip level
    samplerCreateInfo.setAnisotropyEnable(VK_TRUE);                          // Enable Anisotropy; take into account the angle of a surface is being viewed from and decide details based on that (??)

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
}