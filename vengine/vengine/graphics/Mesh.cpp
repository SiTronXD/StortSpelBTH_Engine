#include "pch.h"
#include "Mesh.hpp"
#include "MeshDataModifier.hpp"
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

void Mesh::getLocalBoneTransform(
    const AnimationSlot& aniSlot, const BonePoses& curAnimPose,
    const BonePoses& nextAnimPose, glm::mat4& outputMatrix)
{
    glm::mat4 identityMat(1.0f);

    // Translation
    glm::vec3 translation1;
    glm::vec3 translation2;
    this->getAnimLerp(curAnimPose.translationStamps, aniSlot.timer, translation1);
    this->getAnimLerp(nextAnimPose.translationStamps, aniSlot.nTimer, translation2);
    translation1 = glm::mix(translation1, translation2, aniSlot.alpha);

    // Rotation
    glm::quat rotation1;
    glm::quat rotation2;
    this->getAnimSlerp(curAnimPose.rotationStamps, aniSlot.timer, rotation1);
    this->getAnimSlerp(nextAnimPose.rotationStamps, aniSlot.nTimer, rotation2);
    rotation1 = glm::slerp(rotation1, rotation2, aniSlot.alpha);

    // Scale
    glm::vec3 scale1;
    glm::vec3 scale2;
    this->getAnimLerp(curAnimPose.scaleStamps, aniSlot.timer, scale1);
    this->getAnimLerp(nextAnimPose.scaleStamps, aniSlot.nTimer, scale2);
    scale1 = glm::mix(scale1, scale2, aniSlot.alpha);

    // Final transform
    outputMatrix =
        glm::translate(identityMat, translation1) *
        glm::toMat4(rotation1) *
        glm::scale(identityMat, scale1);
}

Mesh::Mesh(MeshData&& meshData, VulkanImportStructs& importStructs)
    : submeshData(meshData.submeshes), 
    meshData(meshData),
    device(*importStructs.device),
    vma(*importStructs.vma)
{  
    this->createVertexBuffers(meshData, importStructs);
    this->createIndexBuffer( meshData, importStructs);

    // Save a few MB of RAM
    MeshDataModifier::clearVertexStreams(this->meshData);
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
    // Get size of buffer needed for indices
    vk::DeviceSize bufferSize = sizeof(uint32_t) * meshData.indicies.size();

    // Temporary buffer to "stage" index data before transferring to GPU
    vk::Buffer stagingBuffer{};
    VmaAllocation stagingBufferMemory{};
    VmaAllocationInfo allocInfoStaging;

    Buffer::createBuffer(
        {
            .bufferSize     = bufferSize, 
            .bufferUsageFlags = vk::BufferUsageFlagBits::eTransferSrc, 
            .bufferAllocationFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                                | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .buffer         = &stagingBuffer, 
            .bufferMemory   = &stagingBufferMemory,
            .allocationInfo = &allocInfoStaging,
            .vma = importStructs.vma
        }
    );

    // Map Memory to Index Buffer! 
    void* data{}; 
    vmaMapMemory(*importStructs.vma, stagingBufferMemory, &data);
    memcpy(data, meshData.indicies.data(), sizeof(uint32_t) * meshData.indicies.size());
    vmaUnmapMemory(*importStructs.vma, stagingBufferMemory);
    
    VmaAllocationInfo allocInfo_device;

    // Create Buffers for INDEX data on GPU access only area
    Buffer::createBuffer(
        {
            .bufferSize     = bufferSize, 
            .bufferUsageFlags = vk::BufferUsageFlagBits::eTransferDst        // Destination Buffer to be transfered to
                                | vk::BufferUsageFlagBits::eIndexBuffer,     // This is a Index Buffer, will be used as a Index Buffer
            .bufferAllocationFlags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,  // Buffer will be local to the device
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

    // Destroy/free staging buffer resources
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

        const AnimationSlot& aniSlot = animationCompOut.aniSlots[currentBone.slotIndex];
        const Animation& curAnim = this->meshData.animations[aniSlot.animationIndex];

        // if no next animation
        if (aniSlot.nAnimationIndex == ~0u)
        {
            // Start from this local bone transformation
            this->getLocalBoneTransform(curAnim.boneStamps[i], aniSlot.timer, boneTransform);
        }
        else // else blend the two animations
        {
            const Animation& nextAnim = this->meshData.animations[aniSlot.nAnimationIndex];
            this->getLocalBoneTransform(aniSlot,
                curAnim.boneStamps[i],
                nextAnim.boneStamps[i], boneTransform);
        }

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

bool Mesh::isChildOf(const Bone& bone, uint32_t grandpaIndex)
{
    if (bone.parentIndex == grandpaIndex)
    {
        return true;
    }

    if (bone.parentIndex != -1)
    {
        return this->isChildOf(this->meshData.bones[bone.parentIndex], grandpaIndex);
    }

    return false;
}

bool Mesh::createAnimationSlot(const std::string& slotName, const std::string& boneName)
{
    if (this->aniSlots.size() >= NUM_MAX_ANIMATION_SLOTS)
    {
        Log::warning("Mesh::createAnimationSlot | Max animation slots reached");
        return false;
    }
    if (this->aniSlots.count(slotName))
    {
        Log::warning("Mesh::createAnimationSlot | Animation slot \"" + slotName + "\" already exists!");
        return false;
    }

    // Search through the bones for "boneName"
    const uint32_t numBones = (uint32_t)this->meshData.bones.size();
    for (uint32_t i = 0; i < numBones; i++)
    {
        if (this->meshData.bones[i].boneName == boneName)
        {
            const uint32_t slotIdx = (uint32_t)this->aniSlots.size();
            this->aniSlots[slotName] = slotIdx;

            this->meshData.bones[i].slotIndex = slotIdx;
            
            // Loop through the remaining bones to see if they're a child/grand child
            const uint32_t startIndex = i;
            for (uint32_t j = startIndex + 1u; j < numBones; j++)
            {
                Bone& curBone = this->meshData.bones[j];
                if (this->isChildOf(curBone, startIndex))
                {
                    curBone.slotIndex = slotIdx;
                }
            }

            return true;
        }
    }

    Log::error("Mesh::createAnimationSlot | Could not find bone with name \"" + boneName + "\"!");
    return false;
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
#ifdef  _CONSOLE
    if (it == this->aniNames.end())
    {
        Log::error("Could not find animation with name \"" + name + "\"");
    }
#endif //  _CONSOLE

    return it->second;
}

uint32_t Mesh::getAnimationSlotIndex(const std::string& slotName) const
{
    const auto it = this->aniSlots.find(slotName);
#ifdef _CONSOLE
    if (it == this->aniSlots.end())
    {
        Log::error("Mesh::getAnimationSlotIndex | Could not find animation slot with name \"" + slotName + "\"");
    }
#endif

    return it->second;
}

const std::string& Mesh::getAnimationName(uint32_t index) const
{
#ifdef _CONSOLE
    if (index >= this->aniNames.size() || index < 0)
    {
        Log::error("Mesh::getAnimationName | Index \"" + std::to_string(index) + " is invalid\"");
    }
#endif

    for (const auto& name : aniNames)
    {
        if (name.second == index)
        {
            return name.first;
        }
    }

#ifdef _CONSOLE
    Log::error("Mesh::getAnimationName | Failed to find name with index \"" + std::to_string(index) + "\"");
#endif
    return this->aniNames.begin()->first;

    // First solution does not work, I guess it happens since maps doesn't always line up in the order that is expected
    /*auto it = this->aniNames.begin();
    std::advance(it, index);
    return it->first;*/
}

float Mesh::getAnimationEndTime(const std::string& aniName) const
{
    const auto it = this->aniNames.find(aniName);
#ifdef _CONSOLE
    if (it == this->aniSlots.end())
    {
        Log::error("Mesh::getAnimationEndTime | Could not find animation with name \"" + aniName + "\"");
    }
#endif

    return this->meshData.animations[it->second].endTime;
}

float Mesh::getAnimationEndTime(uint32_t index) const
{
#ifdef _CONSOLE
    if (index >= (uint32_t)this->meshData.animations.size())
    {
        Log::error("Mesh::getAnimationEndTime | Invalid index: " + std::to_string(index) +
            ". Num animations: " + std::to_string(this->meshData.animations.size()));
    }
#endif

    return this->meshData.animations[index].endTime;
}

void Mesh::safeCleanup() 
{
    device.waitIdle();
    cleanup();
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