#pragma once 

#include <vulkan/vulkan.hpp>
#include <unordered_map>

#include "../resource_management/ResourceManagerStructs.hpp"
#include "MeshData.hpp"
#include "glm/matrix.hpp"
#include "../graphics/vulkan/VmaUsage.hpp"
#include "ShaderInput.hpp"
#include "VertexBufferArray.hpp"

struct AnimationComponent;

class Mesh
{
private: 
    std::vector<SubmeshData> submeshData;
    MeshData meshData{};
    Device&         device; 
    VmaAllocator&   vma;

    std::unordered_map<std::string, uint32_t> aniNames;
    std::unordered_map<std::string, uint32_t> aniSlots; // slot name - slotIndex

    // One vertex buffer per data stream
    VertexBufferArray vertexBuffers;
    vk::Buffer indexBuffer{};
    VmaAllocation indexBufferMemory{};

    void getAnimLerp(
        const std::vector<std::pair<float, glm::vec3>>& stamps,
        const float& timer,
        glm::vec3& outputValue);
    void getAnimSlerp(
        const std::vector<std::pair<float, glm::quat>>& stamps,
        const float& timer,
        glm::quat& outputValue);

    bool isChildOf(const Bone& bone, uint32_t grandpaIndex);

public:     
    Mesh(MeshData&& meshData, VulkanImportStructs& importStructs);
    Mesh(Mesh&& ref);

    void createVertexBuffers(MeshData& meshData, VulkanImportStructs& importStructs);
    void createIndexBuffer( MeshData& meshData, VulkanImportStructs& importStructs);
    
    void getLocalBoneTransform(
        const BonePoses& bone,
        const float& timer,
        glm::mat4& outputMatrix);

    void getBoneTransforms(AnimationComponent& animationComponentOutput);
    void getLocalBoneTransform(const AnimationSlot& aniSlot, const BonePoses& curAnimPose,
        const BonePoses& nextAnimPose, glm::mat4& outputMatrix);

    inline const VertexBufferArray& getVertexBufferArray() const;
    inline const vk::Buffer& getIndexBuffer() const;
	inline MeshData& getMeshData();
    inline const std::vector<SubmeshData>& getSubmeshData() const;
    inline SubmeshData& getSubmesh(const uint32_t& index) { return this->submeshData[index]; }

    bool createAnimationSlot(const std::string& slotName, const std::string& boneName);
    void mapAnimations(const std::vector<std::string>& names);
    uint32_t getAnimationIndex(const std::string& name) const;
    uint32_t getAnimationSlotIndex(const std::string& slotName) const;
    const std::string& getAnimationName(uint32_t index) const;
    float getAnimationEndTime(const std::string& aniName) const;
    float getAnimationEndTime(uint32_t index) const;

    void safeCleanup();
    void cleanup();

    // Debug
    void outputRigDebugInfo(const std::string& filePath, int animationIndex = 0);
};

const VertexBufferArray& Mesh::getVertexBufferArray() const
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