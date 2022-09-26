#pragma once 

#include <vulkan/vulkan.hpp>

#include "../ResourceManagement/ResourceManagerStructs.hpp"
#include "CommonMeshStructs.hpp"
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
    vk::Buffer& getVertexBuffer();
    vk::Buffer& getIndexBuffer( );
    std::vector<SubmeshData> getSubmeshData();

    void cleanup();
};
