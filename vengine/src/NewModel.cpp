#include "NewModel.hpp"
#include "Utilities.hpp"
#include "tracy/Tracy.hpp"
#include <map>

NewModel::NewModel(MeshData&& meshData, VulkanImportStructs& importStructs)
    : submeshData(meshData.submeshes), device(*importStructs.device),vma(*importStructs.vma)
{  


}



NewModel::NewModel(NewModel&& ref)
:   submeshData(std::move(ref.submeshData)),
    device(ref.device),
    vma(ref.vma),
    vertexBuffer(std::move(ref.vertexBuffer)),
    indexBuffer(std::move(ref.indexBuffer)),
    vertexBufferMemory(std::move(ref.vertexBufferMemory)),
    indexBufferMemory(std::move(ref.indexBufferMemory))
{
}

void NewModel::createVertexBuffer(MeshData& meshData, VulkanImportStructs& importStructs)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif    
    /// Temporary buffer to "Stage" vertex data before transferring to GPU
    vk::Buffer stagingBuffer;    
    VmaAllocation stagingBufferMemory{};

    VmaAllocationInfo allocInfo_staging;

    /// Get size of buffers needed for Vertices
    vk::DeviceSize bufferSize  =sizeof(Vertex) * meshData.vertices.size();

    vengine_helper::createBuffer(
        {
            .physicalDevice = *importStructs.physicalDevice, 
            .device         = importStructs.device->getVkDevice(), 
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

    memcpy(data, meshData.vertices.data(),sizeof(Vertex) * meshData.vertices.size());
    vmaUnmapMemory(*importStructs.vma, stagingBufferMemory); 

    VmaAllocationInfo allocInfo_deviceOnly;
    /// Create Buffer with TRANSFER_DST_BIT to mark as recipient of transfer data (also VERTEX_BUFFER)
    /// Buffer memory is to be DEVICVE_LOCAL_BIT meaning memory is on the GPU and only accesible by it and not the CPU (HOST)
    vengine_helper::createBuffer(
        {
            .physicalDevice = *importStructs.physicalDevice, 
            .device         = importStructs.device->getVkDevice(), 
            .bufferSize     = bufferSize, 
            .bufferUsageFlags = vk::BufferUsageFlagBits::eTransferDst            /// Destination Buffer to be transfered to
                                | vk::BufferUsageFlagBits::eVertexBuffer,    //// This is a Vertex Buffer
            .bufferProperties = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
            .buffer         = &this->vertexBuffer, 
            .bufferMemory   = &this->vertexBufferMemory,
            .allocationInfo = &allocInfo_deviceOnly,
            .vma = importStructs.vma

        });

    /// Copy Staging Buffer to Vertex Buffer on GPU
    vengine_helper::copyBuffer(
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

void NewModel::createIndexBuffer(MeshData& meshData, VulkanImportStructs& importStructs)
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

    vengine_helper::createBuffer(
        {
            .physicalDevice = *importStructs.physicalDevice, 
            .device         = importStructs.device->getVkDevice(), 
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
    vengine_helper::createBuffer(
        {
            .physicalDevice = *importStructs.physicalDevice, 
            .device         = importStructs.device->getVkDevice(), 
            .bufferSize     = bufferSize, 
            .bufferUsageFlags = vk::BufferUsageFlagBits::eTransferDst        /// Destination Buffer to be transfered to
                                | vk::BufferUsageFlagBits::eIndexBuffer,     /// This is a Index Buffer, will be used as a Index Buffer
            .bufferProperties = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,    /// Buffer will be local to the device
            .buffer         = &this->indexBuffer, 
            .bufferMemory   = &this->indexBufferMemory,
            .allocationInfo = &allocInfo_device,
            .vma = importStructs.vma
        });

    vengine_helper::copyBuffer(
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
