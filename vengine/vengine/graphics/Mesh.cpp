#include "pch.h"
#include "Mesh.hpp"
#include "Utilities.hpp"
#include "tracy/Tracy.hpp"
#include "glm/gtx/quaternion.hpp"
#include "Buffer.hpp"
#include "../dev/Log.hpp"

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
    const BonePoses& poses,
    const float& timer,
    const int& animationIndex,
    glm::mat4& outputMatrix)
{
    glm::mat4 identityMat(1.0f);

    // Translation
    glm::vec3 translation;
    this->getAnimLerp(poses.translationStamps, timer, translation);

    // Rotation
    glm::quat rotation;
    this->getAnimSlerp(poses.rotationStamps, timer, rotation);

    // Scale
    glm::vec3 scale;
    this->getAnimLerp(poses.scaleStamps, timer, scale);

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
}

Mesh::Mesh(Mesh&& ref)
:   submeshData(std::move(ref.submeshData)),
    meshData(std::move(ref.meshData)),
    device(ref.device),
    vma(ref.vma),
    vertexBuffers(std::move(ref.vertexBuffers)),
    indexBuffer(std::move(ref.indexBuffer)),
    indexBufferMemory(std::move(ref.indexBufferMemory))
{
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

void Mesh::getBoneTransforms(
    AnimationComponent& animationCompOut)
{
#if defined(_DEBUG) || defined(DEBUG)
    if (this->meshData.bones.size() > NUM_MAX_BONE_TRANSFORMS)
    {
        Log::warning(
            "Mesh data has too many bones (" + std::to_string(this->meshData.bones.size()) + 
            "). A maximum of " + std::to_string(NUM_MAX_BONE_TRANSFORMS) + 
            " bones is allowed. Bone transformations after index " + std::to_string(NUM_MAX_BONE_TRANSFORMS - 1) + 
            " will not be calculated"
        );
    }
#endif

    // Preallocate
    //uint32_t animationIndex = animationCompOut.animationIndex;
    size_t numBones = std::min(
        static_cast<size_t>(NUM_MAX_BONE_TRANSFORMS), 
        this->meshData.bones.size()
    );
    glm::mat4 boneTransform;

    // Loop through bones, from parents to children
    for (size_t i = 0; i < numBones; ++i)
    {
        Bone& currentBone = this->meshData.bones[i];

        // change to animComp.aniSlots[currentBone.slotIndex]
        const AnimationPlayer& aniPlayer = animationCompOut.aniSlots[currentBone.slotIndex]; 
        const Animation& curAnim = this->meshData.animations[aniPlayer.animationIndex];

        // Start from this local bone transformation
        this->getLocalBoneTransform(
            curAnim.boneStamps[i],
            aniPlayer.timer,
            aniPlayer.animationIndex, // TODO: Remove this argument
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
        animationCompOut.boneTransforms[i] =
            currentBone.boneMatrix * currentBone.inverseBindPoseMatrix;
    }

    // Return transformed array
    animationCompOut.numTransforms =
        static_cast<uint32_t>(numBones);
}

bool hasParent(const Bone& bone, const std::vector<Bone>& skeleton, int searchIdx)
{
    if (bone.parentIndex == searchIdx)
    {
        return true;
    }

    if (bone.parentIndex != -1)
    {
        return hasParent(skeleton[bone.parentIndex], skeleton, searchIdx);
    }

    return false;
}

void Mesh::createAnimationSlot(const std::string& slotName, const std::string& boneName)
{
    if (this->aniSlots.size() >= NUM_MAX_ANIMATION_SLOTS)
    {
        Log::warning("Mesh::createAnimationSlot | Max animation slots reached");
        return;
    }
    if (this->aniSlots.count(slotName))
    {
        Log::warning("Mesh::createAnimationSlot | Animation slot \"" + slotName + "\" already exists!");
        return;
    }


    // Rework this
    /*const uint32_t numBones = (uint32_t)this->meshData.bones.size();
    for (uint32_t i = 0; i < numBones; i++)
    {
        if (this->meshData.bones[i].boneName == boneName)
        {
            aniSlots[slotName] = (uint32_t)this->aniSlots.size();
            return;
        }
    }*/

    // To something like:
    /* 
    
        find the bone called boneName
        {
            set Bone::slotIndex to (uint32_t)this->aniSlots.size();
            for all children
                set Bone::slotIndex to (uint32_t)this->aniSlots.size();
        }

    */

    // Temp
    /*const uint32_t slotIdx = (uint32_t)this->aniSlots.size();
    this->aniSlots[slotName] = slotIdx;
    return;*/

    const int numBones = (int)this->meshData.bones.size();
    for (int i = 0; i < numBones; i++)
    {
        if (this->meshData.bones[i].boneName == boneName)
        {
            const uint32_t slotIdx = (uint32_t)this->aniSlots.size() + 1u;
            this->aniSlots[slotName] = slotIdx;

            this->meshData.bones[i].slotIndex = slotIdx;
            
            const int startIndex = i;
            for (size_t j = startIndex + 1ull; j < numBones; j++)
            {
                Bone& curBone = this->meshData.bones[j];
                if (hasParent(curBone, this->meshData.bones, startIndex))
                {
                    curBone.slotIndex = slotIdx;
                }
            }

            return;
        }
    }

    Log::warning("Mesh::createAnimationSlot | Could not find bone with name \"" + boneName + "\"!");
}

void Mesh::mapAnimations(const std::vector<std::string>& names)
{
    this->aniNames.clear();
    uint32_t index = 0;
    for (const std::string& name : names)
    {
        this->aniNames[name] = index++;
    }
}

uint32_t Mesh::getAnimationIndex(const std::string& name) const
{
    const auto it = this->aniNames.find(name);
    if (it == this->aniNames.end())
    {
        Log::error("Could not find animation with name \"" + name + "\"");
    }

    return it->second;
}

uint32_t Mesh::getAnimationSlotIndex(const std::string& slotName) const
{
    const auto it = this->aniSlots.find(slotName);
    if (it == this->aniSlots.end())
    {
        Log::error("Could not find animation with name \"" + slotName + "\"");
    }
    
    return it->second;
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
    const Animation& animation = this->meshData.animations[animationIndex];
    for (size_t i = 0; i < this->meshData.bones.size(); ++i)
    {
        const BonePoses& poses = animation.boneStamps[i];

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
        file << "translations (" << poses.translationStamps.size() << "): " << std::endl;
        for (size_t j = 0; j < poses.translationStamps.size(); ++j)
        {
            file << "[" << j << "] [" <<
                poses.translationStamps[j].first << "](" <<
                poses.translationStamps[j].second.x << ", " <<
                poses.translationStamps[j].second.y << ", " <<
                poses.translationStamps[j].second.z << ")" <<
                std::endl;
        }

        file << "rotation (" << poses.rotationStamps.size() << "): " << std::endl;
        for (size_t j = 0; j < poses.rotationStamps.size(); ++j)
        {
            file << "[" << j << "] [" <<
                poses.rotationStamps[j].first << "](" <<
                poses.rotationStamps[j].second.x << ", " <<
                poses.rotationStamps[j].second.y << ", " <<
                poses.rotationStamps[j].second.z << ", " <<
                poses.rotationStamps[j].second.w << ")" <<
                std::endl;
        }

        file << "scale (" << poses.scaleStamps.size() << "): " << std::endl;
        for (size_t j = 0; j < poses.scaleStamps.size(); ++j)
        {
            file << "[" << j << "] [" <<
                poses.scaleStamps[j].first << "](" <<
                poses.scaleStamps[j].second.x << ", " <<
                poses.scaleStamps[j].second.y << ", " <<
                poses.scaleStamps[j].second.z << ")" <<
                std::endl;
        }

        file << std::endl;
    }
    file << std::endl;

    // Close file
    file.close();
#endif
}