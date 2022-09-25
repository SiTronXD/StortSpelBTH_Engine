#pragma once 

#include <vulkan/vulkan.hpp>

#include "ResourceManagerStructs.hpp"
#include "CommonMeshStructs.hpp"
#include "glm/matrix.hpp"
#include "vk_mem_alloc.h"


class NewModel{
private: 
    std::vector<SubmeshData>    submeshData;
    Device&         device; 
    VmaAllocator&   vma;     

    vk::Buffer  vertexBuffer{};
    vk::Buffer  indexBuffer{};
    VmaAllocation vertexBufferMemory{};
    VmaAllocation indexBufferMemory{};
public:     
    NewModel(MeshData&& meshData, VulkanImportStructs& importStructs);
    NewModel(NewModel&& ref);
    void createVertexBuffer(MeshData& meshData, VulkanImportStructs& importStructs);
    void createIndexBuffer( MeshData& meshData, VulkanImportStructs& importStructs);
    vk::Buffer& getVertexBuffer();
    vk::Buffer& getIndexBuffer( );
    std::vector<SubmeshData> getSubmeshData();

    void cleanup();
};
