#include "Mesh.hpp"
#include "Utilities.hpp"
#include "tracy/Tracy.hpp"
#include "glm/gtx/quaternion.hpp"
#include "Buffer.hpp"
#include <map>

void Mesh::getAnimLerp(
    const std::vector<std::pair<float, glm::vec3>>& stamps,
    const float& timer,
    glm::vec3& outputValue
)
{
    // Init to last value
    outputValue = stamps[stamps.size() - 1].second;

    // Try to find interval
    for (size_t i = 0; i < stamps.size() - 1; ++i)
    {
        float time1 = stamps[i + 1].first;

        // Found interval
        if (timer < time1)
        {
            float time0 = stamps[i].first;
            float alpha = (timer - time0) / (time1 - time0);

            // Interpolate
            outputValue = glm::mix(
                stamps[i].second,
                stamps[i + 1].second,
                alpha);

            break;
        }
    }
}

void Mesh::getAnimSlerp(
    const std::vector<std::pair<float, glm::quat>>& stamps,
    const float& timer,
    glm::quat& outputValue
)
{
    // Init to last value
    outputValue = stamps[stamps.size() - 1].second;

    // Try to find interval
    for (size_t i = 0; i < stamps.size() - 1; ++i)
    {
        float time1 = stamps[i + 1].first;

        // Found interval
        if (timer < time1)
        {
            float time0 = stamps[i].first;
            float alpha = (timer - time0) / (time1 - time0);

            // Interpolate
            outputValue =
                glm::slerp(
                    stamps[i].second,
                    stamps[i + 1].second,
                    alpha);

            break;
        }
    }
}

void Mesh::getLocalBoneTransform(
    const Bone& bone,
    const float& timer,
    const int& animationIndex,
    glm::mat4& outputMatrix)
{
    glm::mat4 identityMat(1.0f);

    // Translation
    glm::vec3 translation;
    this->getAnimLerp(bone.animations[animationIndex].translationStamps, timer, translation);

    // Rotation
    glm::quat rotation;
    this->getAnimSlerp(bone.animations[animationIndex].rotationStamps, timer, rotation);

    // Scale
    glm::vec3 scale;
    this->getAnimLerp(bone.animations[animationIndex].scaleStamps, timer, scale);

    // Final transform
    outputMatrix =
        glm::translate(identityMat, translation) *
        glm::toMat4(rotation) *
        glm::scale(identityMat, scale);
}

Mesh::Mesh(MeshData&& meshData, VulkanImportStructs& importStructs)
    : submeshData(meshData.submeshes), 
    meshData(meshData),
    device(*importStructs.device),
    vma(*importStructs.vma)
{  
    this->createVertexBuffers(meshData, importStructs);
    this->createIndexBuffer( meshData, importStructs);

    this->boneTransforms.resize(meshData.bones.size());
}

Mesh::Mesh(Mesh&& ref)
:   submeshData(std::move(ref.submeshData)),
    meshData(std::move(ref.meshData)),
    device(ref.device),
    vma(ref.vma),
    boneTransforms(std::move(ref.boneTransforms)),
    vertexBuffers(std::move(ref.vertexBuffers)),
    indexBuffer(std::move(ref.indexBuffer)),
    indexBufferMemory(std::move(ref.indexBufferMemory))
{
    this->boneTransforms.resize(meshData.bones.size());
}

void Mesh::createVertexBuffers(
    MeshData& meshData, 
    VulkanImportStructs& importStructs)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif    
    
    // Ready array for vertex buffers
    this->vertexBuffers.create(
        *importStructs.device, 
        *importStructs.vma,
        *importStructs.transferQueue,
        *importStructs.transferCommandPool
    );

    // Create one vertex buffer per data stream
    
    // Positions
    this->vertexBuffers.addVertexBuffer(
        meshData.vertexStreams.positions
    );

    // Normals
    this->vertexBuffers.addVertexBuffer(
        meshData.vertexStreams.normals
    );

    // Colors
    this->vertexBuffers.addVertexBuffer(
        meshData.vertexStreams.colors
    );

    // Texture coordinates
    this->vertexBuffers.addVertexBuffer(
        meshData.vertexStreams.texCoords
    );

    // Bone weights
    this->vertexBuffers.addVertexBuffer(
        meshData.vertexStreams.boneWeights
    );

    // Bone indices
    this->vertexBuffers.addVertexBuffer(
        meshData.vertexStreams.boneIndices
    );
}

void Mesh::createIndexBuffer(MeshData& meshData, VulkanImportStructs& importStructs)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    /// Get size of buffer needed for indices
    vk::DeviceSize bufferSize = sizeof(uint32_t) * meshData.indicies.size();

    /// Temporary buffer to "Stage" index data before transferring to GPU
    vk::Buffer stagingBuffer{};
    VmaAllocation stagingBufferMemory{};
    VmaAllocationInfo allocInfo_staging;

    Buffer::createBuffer(
        {
            .bufferSize     = bufferSize, 
            .bufferUsageFlags = vk::BufferUsageFlagBits::eTransferSrc, 
            .bufferProperties = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                                | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .buffer         = &stagingBuffer, 
            .bufferMemory   = &stagingBufferMemory,
            .allocationInfo = &allocInfo_staging,
            .vma = importStructs.vma
        });

    /// Map Memory to Index Buffer! 
    void* data{}; 
    vmaMapMemory(*importStructs.vma, stagingBufferMemory, &data);
    memcpy(data, meshData.indicies.data(), sizeof(uint32_t) * meshData.indicies.size());
    vmaUnmapMemory(*importStructs.vma, stagingBufferMemory);
    
    VmaAllocationInfo allocInfo_device;

    /// Create Buffers for INDEX data on GPU access only area
    Buffer::createBuffer(
        {
            .bufferSize     = bufferSize, 
            .bufferUsageFlags = vk::BufferUsageFlagBits::eTransferDst        /// Destination Buffer to be transfered to
                                | vk::BufferUsageFlagBits::eIndexBuffer,     /// This is a Index Buffer, will be used as a Index Buffer
            .bufferProperties = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,  /// Buffer will be local to the device
            .buffer         = &this->indexBuffer, 
            .bufferMemory   = &this->indexBufferMemory,
            .allocationInfo = &allocInfo_device,
            .vma = importStructs.vma
        });

    Buffer::copyBuffer(
        importStructs.device->getVkDevice(), 
        *importStructs.transferQueue, 
        *importStructs.transferCommandPool, 
        stagingBuffer, 
        this->indexBuffer, 
        bufferSize);

    /// Destroy + Release Staging Buffer resources
    importStructs.device->getVkDevice().destroyBuffer(stagingBuffer);
    vmaFreeMemory(*importStructs.vma,stagingBufferMemory);
}

const std::vector<glm::mat4>& Mesh::getBoneTransforms(const float& timer, const int& animationIndex)
{
    // Preallocate
    glm::mat4 boneTransform;

    // Loop through bones, from parents to children
    for (size_t i = 0; i < this->meshData.bones.size(); ++i)
    {
        Bone& currentBone = this->meshData.bones[i];

        // Start from this local bone transformation
        this->getLocalBoneTransform(
            currentBone,
            timer,
            animationIndex,
            boneTransform
        );

        // Apply parent transform if it exists
        if (currentBone.parentIndex >= 0)
        {
            boneTransform =
                this->meshData.bones[currentBone.parentIndex].boneMatrix * 
                boneTransform;
        }

        // Set bone transform for children to reuse
        currentBone.boneMatrix = boneTransform;

        // Apply ((parent * local) * inverseBind)
        this->boneTransforms[i] = 
            currentBone.boneMatrix * currentBone.inverseBindPoseMatrix;
    }

    // Return transformed array
    return this->boneTransforms;
}

void Mesh::cleanup()
{
    this->vertexBuffers.cleanup();

    this->device.getVkDevice().destroyBuffer(this->indexBuffer);
    vmaFreeMemory(this->vma, this->indexBufferMemory);
}

#include <fstream>
#include <iostream>
void Mesh::outputRigDebugInfo(const std::string& filePath, int animationIndex)
{
#if defined(_DEBUG) || defined(DEBUG)
    // Create file
    std::ofstream file(filePath);
    
    // Write
    for (size_t i = 0; i < this->meshData.bones.size(); ++i)
    {
        Animation& animation = this->meshData.bones[i].animations[animationIndex];

        file << "bone [" << i << "]: " << std::endl;
        file << "InvBindPose: ";
        for (uint32_t a = 0; a < 4; ++a)
        {
            for (uint32_t b = 0; b < 4; ++b)
            {
                file << this->meshData.bones[i].inverseBindPoseMatrix[a][b] << " ";
            }
        }
        file << std::endl;
        file << "translations (" << animation.translationStamps.size() << "): " << std::endl;
        for (size_t j = 0; j < animation.translationStamps.size(); ++j)
        {
            file << "[" << j << "] [" <<
                animation.translationStamps[j].first << "](" <<
                animation.translationStamps[j].second.x << ", " <<
                animation.translationStamps[j].second.y << ", " <<
                animation.translationStamps[j].second.z << ")" <<
                std::endl;
        }

        file << "rotation (" << animation.rotationStamps.size() << "): " << std::endl;
        for (size_t j = 0; j < animation.rotationStamps.size(); ++j)
        {
            file << "[" << j << "] [" <<
                animation.rotationStamps[j].first << "](" <<
                animation.rotationStamps[j].second.x << ", " <<
                animation.rotationStamps[j].second.y << ", " <<
                animation.rotationStamps[j].second.z << ", " <<
                animation.rotationStamps[j].second.w << ")" <<
                std::endl;
        }

        file << "scale (" << animation.scaleStamps.size() << "): " << std::endl;
        for (size_t j = 0; j < animation.scaleStamps.size(); ++j)
        {
            file << "[" << j << "] [" <<
                animation.scaleStamps[j].first << "](" <<
                animation.scaleStamps[j].second.x << ", " <<
                animation.scaleStamps[j].second.y << ", " <<
                animation.scaleStamps[j].second.z << ")" <<
                std::endl;
        }

        file << std::endl;
    }
    file << std::endl;

    // Close file
    file.close();
#endif
}