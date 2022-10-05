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

glm::mat4 Mesh::getLocalBoneTransform(
    const Bone& bone,
    const float& timer)
{
    // Translation
    glm::vec3 translation;
    this->getAnimLerp(bone.translationStamps, timer, translation);

    // Rotation
    glm::quat rotation;
    this->getAnimSlerp(bone.rotationStamps, timer, rotation);

    // Scale
    glm::vec3 scale;
    this->getAnimLerp(bone.scaleStamps, timer, scale);

    // Final transform
    glm::mat4 finalTransform =
        glm::translate(glm::mat4(1.0f), translation) *
        glm::toMat4(rotation) *
        glm::scale(glm::mat4(1.0f), scale);

    return finalTransform;
}

Mesh::Mesh(MeshData&& meshData, VulkanImportStructs& importStructs)
    : submeshData(meshData.submeshes), 
    meshData(meshData),
    device(*importStructs.device),
    vma(*importStructs.vma)
{  
    this->createVertexBuffer(meshData, importStructs);
    this->createIndexBuffer( meshData, importStructs);

    this->boneTransforms.resize(meshData.bones.size());
}

Mesh::Mesh(Mesh&& ref)
:   submeshData(std::move(ref.submeshData)),
    meshData(std::move(ref.meshData)),
    device(ref.device),
    vma(ref.vma),
    vertexBuffer(std::move(ref.vertexBuffer)),
    indexBuffer(std::move(ref.indexBuffer)),
    vertexBufferMemory(std::move(ref.vertexBufferMemory)),
    indexBufferMemory(std::move(ref.indexBufferMemory))
{
    this->boneTransforms.resize(meshData.bones.size());
}

void Mesh::createVertexBuffer(MeshData& meshData, VulkanImportStructs& importStructs)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif    
    /// Temporary buffer to "Stage" vertex data before transferring to GPU
    vk::Buffer stagingBuffer;    
    VmaAllocation stagingBufferMemory{};

    VmaAllocationInfo allocInfo_staging;

    /// Get size of buffers needed for Vertices
    bool hasAnimations = meshData.aniVertices.size() > 0;

    vk::DeviceSize bufferSize =
        !hasAnimations ? 
        sizeof(meshData.vertices[0]) * meshData.vertices.size() : 
        sizeof(meshData.aniVertices[0]) * meshData.aniVertices.size();

    Buffer::createBuffer(
        {
            .bufferSize     = bufferSize,  
            .bufferUsageFlags = vk::BufferUsageFlagBits::eTransferSrc,       /// This buffers vertex data will be transfered somewhere else!
            .bufferProperties = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                                | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .buffer         = &stagingBuffer,
            .bufferMemory   = &stagingBufferMemory,
            .allocationInfo = &allocInfo_staging,
            .vma = importStructs.vma
        });    
    
    /// -- Map memory to our Temporary Staging Vertex Buffer -- 
    void * data{};
    if(vmaMapMemory(*importStructs.vma, stagingBufferMemory, &data) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate Mesh Staging Vertex Buffer Using VMA!");
    };

    if (!hasAnimations)
    {
        memcpy(
            data,
            meshData.vertices.data(),
            (size_t) bufferSize
        );
    }
    else
    {
        memcpy(
            data,
            meshData.aniVertices.data(), 
            (size_t) bufferSize
        );
    }
    
    vmaUnmapMemory(*importStructs.vma, stagingBufferMemory); 

    VmaAllocationInfo allocInfo_deviceOnly;
    /// Create Buffer with TRANSFER_DST_BIT to mark as recipient of transfer data (also VERTEX_BUFFER)
    /// Buffer memory is to be DEVICVE_LOCAL_BIT meaning memory is on the GPU and only accesible by it and not the CPU (HOST)
    Buffer::createBuffer(
        {
            .bufferSize     = bufferSize, 
            .bufferUsageFlags = vk::BufferUsageFlagBits::eTransferDst        /// Destination Buffer to be transfered to
                                | vk::BufferUsageFlagBits::eVertexBuffer,    //// This is a Vertex Buffer
            .bufferProperties = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
            .buffer         = &this->vertexBuffer, 
            .bufferMemory   = &this->vertexBufferMemory,
            .allocationInfo = &allocInfo_deviceOnly,
            .vma = importStructs.vma

        });

    /// Copy Staging Buffer to Vertex Buffer on GPU
    Buffer::copyBuffer(
        importStructs.device->getVkDevice(), 
        *importStructs.transferQueue, 
        *importStructs.transferCommandPool, 
        stagingBuffer, 
        this->vertexBuffer, 
        bufferSize);

    /// Clean up Staging Buffer stuff
    importStructs.device->getVkDevice().destroyBuffer(stagingBuffer);
    vmaFreeMemory(*importStructs.vma, stagingBufferMemory);
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

const std::vector<glm::mat4>& Mesh::getBoneTransforms(const float& timer)
{
    // Loop through bones, from parents to children
    for (size_t i = 0; i < this->meshData.bones.size(); ++i)
    {
        Bone& currentBone = this->meshData.bones[i];

        // Reset to identity matrix
        glm::mat4 finalTransform =
            currentBone.inverseBindPoseMatrix;
        glm::mat4 deltaTransform = glm::mat4(1.0f);

        // Apply transformation from parent
        if (currentBone.parentIndex >= 0)
        {
            deltaTransform =
                this->meshData.bones[currentBone.parentIndex].finalMatrix;
        }

        // Apply this local transformation
        deltaTransform =
            deltaTransform *
            this->getLocalBoneTransform(
                currentBone,
                timer
            );

        currentBone.finalMatrix = deltaTransform;

        // Apply inverse bind and local transform
        finalTransform =
            deltaTransform * finalTransform;

        // Apply final transform to array element
        this->boneTransforms[i] = finalTransform;
    }

    return this->boneTransforms;
}

void Mesh::cleanup()
{
    this->device.getVkDevice().destroyBuffer(this->vertexBuffer);
    this->device.getVkDevice().destroyBuffer(this->indexBuffer);
    vmaFreeMemory(this->vma, this->indexBufferMemory);
    vmaFreeMemory(this->vma, this->vertexBufferMemory);
}
