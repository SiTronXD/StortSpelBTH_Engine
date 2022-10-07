#pragma once 

#include <vulkan/vulkan.hpp>

#include "../resource_management/ResourceManagerStructs.hpp"
#include "MeshData.hpp"
#include "glm/matrix.hpp"
#include "../graphics/vulkan/VmaUsage.hpp"


class Mesh{
private:
    std::vector<SubmeshData>    submeshData;
    Device&         device; 
    VmaAllocator&   vma;     

    vk::Buffer  vertexBuffer{};
    vk::Buffer  indexBuffer{};
    VmaAllocation vertexBufferMemory{};
    VmaAllocation indexBufferMemory{};
public:
    Mesh(MeshData&& meshData, VulkanImportStructs& importStructs);
    Mesh(Mesh&& ref);
    void createVertexBuffer(MeshData& meshData, VulkanImportStructs& importStructs);
    void createIndexBuffer( MeshData& meshData, VulkanImportStructs& importStructs);
    inline const vk::Buffer& getVertexBuffer();
    inline const vk::Buffer& getIndexBuffer( );
    inline const std::vector<SubmeshData>& getSubmeshData();

    void cleanup();
};

const vk::Buffer& Mesh::getVertexBuffer()
{
    return this->vertexBuffer;
}

const vk::Buffer& Mesh::getIndexBuffer()
{
    return this->indexBuffer;
}

const std::vector<SubmeshData>& Mesh::getSubmeshData()
{
    return this->submeshData;
}