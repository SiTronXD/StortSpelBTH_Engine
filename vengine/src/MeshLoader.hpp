#pragma once



/// TODO: Mesh loader wwill be abstract interface, will know anything about assimp implementation
#include "assimp/Importer.hpp"

#include "vk_mem_alloc.h"
#include "Mesh.hpp"
#include "Model.hpp"

#include <vulkan/vulkan.hpp>

/// TODO: not a real class, just something to start with...
class MeshLoader{
public: 
    static Assimp::Importer     importer;
    static VmaAllocator*        vma; 
    static vk::PhysicalDevice*  physicalDevice; 
    static vk::Device*          device; 
    static vk::Queue*           transferQueue; 
    static vk::CommandPool*     transferCommandPool; 
    static Model&& createMesh(std::string path);
};