#include "MeshLoader.hpp"

#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "Configurator.hpp"



Assimp::Importer MeshLoader::importer; 
VulkanImportStructs MeshLoader::importStructs;

void MeshLoader::init(VmaAllocator *vma,
                                     vk::PhysicalDevice *physiscalDev,
                                     Device *dev, vk::Queue *transQueue,
                                     vk::CommandPool *transCmdPool) {
  MeshLoader::importStructs.vma = vma;
  MeshLoader::importStructs.physicalDevice = physiscalDev;
  MeshLoader::importStructs.device = dev;
  MeshLoader::importStructs.transferQueue = transQueue;
  MeshLoader::importStructs.transferCommandPool = transCmdPool;
}

