#pragma once 

#include <vulkan/vulkan.hpp>

#include "../ResourceManagement/ResourceManagerStructs.hpp"
#include "MeshData.hpp"
#include "glm/matrix.hpp"
#include "../graphics/vulkan/VmaUsage.hpp"
#include "ShaderInput.hpp"

class Mesh
{
private: 
    std::vector<SubmeshData> submeshData;
    MeshData meshData;
    Device&         device; 
    VmaAllocator&   vma;

    std::vector<glm::mat4> boneTransforms;

    // One vertex buffer per data stream
    std::vector<vk::DeviceSize> vertexBufferOffsets;
    std::vector<vk::Buffer> vertexBuffers;
    std::vector<VmaAllocation> vertexBufferMemories;
    vk::Buffer indexBuffer{};
    VmaAllocation indexBufferMemory{};

    template <typename T>
    void createSeparateVertexBuffer(
        const std::vector<T>& dataStream,
        const VulkanImportStructs& importStructs);

    void getAnimLerp(
        const std::vector<std::pair<float, glm::vec3>>& stamps,
        const float& timer,
        glm::vec3& outputValue);
    void getAnimSlerp(
        const std::vector<std::pair<float, glm::quat>>& stamps,
        const float& timer,
        glm::quat& outputValue);
    void getLocalBoneTransform(
        const Bone& bone,
        const float& timer,
        glm::mat4& outputMatrix
    );

public:     
    Mesh(MeshData&& meshData, VulkanImportStructs& importStructs);
    Mesh(Mesh&& ref);
    
    void createVertexBuffers(MeshData& meshData, VulkanImportStructs& importStructs);
    void createIndexBuffer( MeshData& meshData, VulkanImportStructs& importStructs);
    
    const std::vector<glm::mat4>& getBoneTransforms(const float& timer);

    inline const std::vector<vk::DeviceSize>& getVertexBufferOffsets() const;
    inline const std::vector<vk::Buffer>& getVertexBuffers() const;
    inline const vk::Buffer& getIndexBuffer() const;
	inline MeshData& getMeshData();
    inline const std::vector<SubmeshData>& getSubmeshData() const;
    
    void cleanup();
};

const std::vector<vk::DeviceSize>& Mesh::getVertexBufferOffsets() const
{
    return this->vertexBufferOffsets;
}

const std::vector<vk::Buffer>& Mesh::getVertexBuffers() const
{
    return this->vertexBuffers;
}

const vk::Buffer& Mesh::getIndexBuffer() const
{
    return this->indexBuffer;
}

MeshData& Mesh::getMeshData()
{
	return this->meshData;
}

const std::vector<SubmeshData>& Mesh::getSubmeshData() const
{
    return this->submeshData;
}