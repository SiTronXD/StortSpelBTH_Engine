#include "TextureLoader.hpp"

/// TODO: REMOVE TEMP and VulkanRender (temporary solution for checking
/// createTedxture...)
#include "VulkanRenderer.hpp"

VulkanImportStructs TextureLoader::importStructs;
VulkanRenderer *TextureLoader::TEMP = nullptr; 


void TextureLoader::init(VmaAllocator *vma,
                                        vk::PhysicalDevice *physiscalDev,
                                        Device *dev, vk::Queue *transQueue,
                                        vk::CommandPool *transCmdPool) {
  TextureLoader::importStructs.vma = vma;
  TextureLoader::importStructs.physicalDevice = physiscalDev;
  TextureLoader::importStructs.device = dev;
  TextureLoader::importStructs.transferQueue = transQueue;
  TextureLoader::importStructs.transferCommandPool = transCmdPool;
}
