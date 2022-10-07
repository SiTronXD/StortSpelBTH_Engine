#include "Mesh.hpp"
#include "Utilities.hpp"
#include "tracy/Tracy.hpp"
#include "glm/gtx/quaternion.hpp"
#include "Buffer.hpp"
#include <map>

template <typename T>
void Mesh::createSeparateVertexBuffer(
    const std::vector<T>& dataStream,
    const VulkanImportStructs& importStructs)
{
    this->vertexBuffers.push_back(vk::Buffer());
    this->vertexBufferMemories.push_back(VmaAllocation());

    /// Temporary buffer to "Stage" vertex data before transferring to GPU
    vk::Buffer stagingBuffer;    
    VmaAllocation stagingBufferMemory{};
    VmaAllocationInfo allocInfo_staging;

    vk::DeviceSize bufferSize = sizeof(dataStream[0]) * dataStream.size();

    Buffer::createBuffer(
        {
            .bufferSize = bufferSize,
            .bufferUsageFlags = vk::BufferUsageFlagBits::eTransferSrc,       /// This buffers vertex data will be transfered somewhere else!
            .bufferProperties = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                                | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .buffer = &stagingBuffer,
            .bufferMemory = &stagingBufferMemory,
            .allocationInfo = &allocInfo_staging,
            .vma = importStructs.vma
        });

    /// -- Map memory to our Temporary Staging Vertex Buffer -- 
    void* data{};
    if (vmaMapMemory(*importStructs.vma, stagingBufferMemory, &data) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate Mesh Staging Vertex Buffer Using VMA!");
    };

    memcpy(
        data,
        dataStream.data(),
        (size_t)bufferSize
    );

    /*if (!hasAnimations)
    {
        memcpy(
            data,
            meshData.vertices.data(),
            (size_t)bufferSize
        );
    }
    else
    {
        memcpy(
            data,
            meshData.aniVertices.data(),
            (size_t)bufferSize
        );
    }*/

    vmaUnmapMemory(*importStructs.vma, stagingBufferMemory);

    VmaAllocationInfo allocInfo_deviceOnly;
    /// Create Buffer with TRANSFER_DST_BIT to mark as recipient of transfer data (also VERTEX_BUFFER)
    /// Buffer memory is to be DEVICVE_LOCAL_BIT meaning memory is on the GPU and only accesible by it and not the CPU (HOST)
    Buffer::createBuffer(
        {
            .bufferSize = bufferSize,
            .bufferUsageFlags = vk::BufferUsageFlagBits::eTransferDst        /// Destination Buffer to be transfered to
                                | vk::BufferUsageFlagBits::eVertexBuffer,    //// This is a Vertex Buffer
            .bufferProperties = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
            .buffer = &this->vertexBuffers[this->vertexBuffers.size() - 1],
            .bufferMemory = &this->vertexBufferMemories[this->vertexBufferMemories.size() - 1],
            .allocationInfo = &allocInfo_deviceOnly,
            .vma = importStructs.vma

        });

    /// Copy Staging Buffer to Vertex Buffer on GPU
    Buffer::copyBuffer(
        importStructs.device->getVkDevice(),
        *importStructs.transferQueue,
        *importStructs.transferCommandPool,
        stagingBuffer,
        this->vertexBuffers[this->vertexBuffers.size() - 1],
        bufferSize);

    /// Clean up Staging Buffer stuff
    importStructs.device->getVkDevice().destroyBuffer(stagingBuffer);
    vmaFreeMemory(*importStructs.vma, stagingBufferMemory);
}

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
    glm::mat4& outputMatrix)
{
    glm::mat4 identityMat(1.0f);

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
    vertexBufferOffsets(std::move(ref.vertexBufferOffsets)),
    vertexBuffers(std::move(ref.vertexBuffers)),
    indexBuffer(std::move(ref.indexBuffer)),
    vertexBufferMemories(std::move(ref.vertexBufferMemories)),
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
    
    /// Get size of buffers needed for Vertices
    size_t numVerts = meshData.vertices.size();
    const AvailableVertexData& availableVertexData = 
        meshData.availableVertexData;

    uint32_t vertexBufferIndex = 0;

    // Create one vertex buffer per data stream
    if (availableVertexData & (uint32_t)VertexData::POSITION)
    {
        std::vector<glm::vec3> positions(numVerts);
        for (size_t i = 0; i < numVerts; ++i)
            positions[i] = meshData.vertices[i].pos;
        this->createSeparateVertexBuffer(positions, importStructs);
    }

    if (availableVertexData & (uint32_t)VertexData::COLOR)
    {
        std::vector<glm::vec3> colors(numVerts);
        for (size_t i = 0; i < numVerts; ++i)
            colors[i] = meshData.vertices[i].col;
        this->createSeparateVertexBuffer(colors, importStructs);
    }

    if (availableVertexData & (uint32_t)VertexData::TEX_COORDS)
    {
        std::vector<glm::vec2> texCoords(numVerts);
        for (size_t i = 0; i < numVerts; ++i)
            texCoords[i] = meshData.vertices[i].tex;
        this->createSeparateVertexBuffer(texCoords, importStructs);
    }
    
    if (availableVertexData & (uint32_t)VertexData::BONE_WEIGHTS)
    {
        std::vector<glm::vec4> boneWeights(numVerts);
        for (size_t i = 0; i < numVerts; ++i)
            boneWeights[i] = glm::vec4(
                meshData.vertices[i].weights[0],
                meshData.vertices[i].weights[1],
                meshData.vertices[i].weights[2],
                meshData.vertices[i].weights[3]);
        this->createSeparateVertexBuffer(boneWeights, importStructs);
    }

    if (availableVertexData & (uint32_t)VertexData::BONE_INDICES)
    {
        std::vector<glm::uvec4> boneIndices(numVerts);
        for (size_t i = 0; i < numVerts; ++i)
            boneIndices[i] = glm::uvec4(
                meshData.vertices[i].bonesIndex[0],
                meshData.vertices[i].bonesIndex[1],
                meshData.vertices[i].bonesIndex[2],
                meshData.vertices[i].bonesIndex[3]);
        this->createSeparateVertexBuffer(boneIndices, importStructs);
    }

    // Vertex buffer offsets when binding
    this->vertexBufferOffsets.resize(this->vertexBuffers.size());
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
    for (size_t i = 0; i < this->vertexBuffers.size(); ++i)
    {
        this->device.getVkDevice().destroyBuffer(this->vertexBuffers[i]);
        vmaFreeMemory(this->vma, this->vertexBufferMemories[i]);
    }
    this->device.getVkDevice().destroyBuffer(this->indexBuffer);
    vmaFreeMemory(this->vma, this->indexBufferMemory);
}
