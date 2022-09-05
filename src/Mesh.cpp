#include "Mesh.h"
#include "Utilities.h"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <exception>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
#include "vk_mem_alloc.h"
#include "tracy/Tracy.hpp"

Mesh::Mesh(VmaAllocator *vma,
        vk::PhysicalDevice newPhysicalDevice, 
        vk::Device newDevice,
        vk::Queue transferQueue, 
        vk::CommandPool transferCommandPool, 
        std::vector<Vertex> * vertices, 
        std::vector<uint32_t> * indicies,
        int newTextureID)
    :   model(),  
        texId(newTextureID), 
        vma(vma),
        vertices(*vertices),
        indicies(*indicies),
        vertexCount(vertices->size()),
        indexCount(indicies->size()),
        physicalDevice(newPhysicalDevice), 
        device(newDevice),
        transferQueue(transferQueue),
        transferCommandPool(transferCommandPool)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    //this->createVertexBuffer(vertices);
    //this->createIndexBuffer(indicies);
    this->model.model = glm::mat4(1.F);
}

void Mesh::setModelMatrix(glm::mat4 newModel)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    this->model.model = newModel;
}

ModelMatrix* Mesh::getModelMatrix()
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    return &this->model;
}

int Mesh::getTextureId() const
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    return this->texId;
}

uint32_t Mesh::getVertexCount() const
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    return this->vertexCount;
}

vk::Buffer Mesh::getVertexBuffer()
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    return vertexBuffer;
}

std::vector<Vertex>& Mesh::getVertex_vector()
{
    return this->vertices;
}

std::vector<uint32_t>& Mesh::getIndicies_vector()
{
    return this->indicies;
}

uint32_t Mesh::getIndexCount() const
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    return indexCount;
}

vk::Buffer Mesh::getIndexBuffer()
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    return this->indexBuffer;
}

void Mesh::destroyBuffers()
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    this->device.destroyBuffer(this->vertexBuffer);
    //this->device.freeMemory(this->vertexBufferMemory);
    vmaFreeMemory(*this->vma,this->vertexBufferMemory);
    this->device.destroyBuffer(indexBuffer);
    //this->device.freeMemory(this->indexBufferMemory);
    vmaFreeMemory(*this->vma,this->indexBufferMemory);
}

void Mesh::createVertexBuffer(std::vector<Vertex>* vertices)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    /// Temporary buffer to "Stage" vertex data before transferring to GPU
    vk::Buffer stagingBuffer;
    //vk::DeviceMemory stagingBufferMemory;
    VmaAllocation stagingBufferMemory{};

    VmaAllocationInfo allocInfo_staging;

    /// Get size of buffers needed for Vertices
    vk::DeviceSize bufferSize  =sizeof(Vertex) * vertices->size();

    vengine_helper::createBuffer(
        {
            .physicalDevice = physicalDevice, 
            .device         = device, 
            .bufferSize     = bufferSize, 
            ///.bufferUsageFlags =  VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
            .bufferUsageFlags = vk::BufferUsageFlagBits::eTransferSrc,       /// This buffers vertex data will be transfered somewhere else!
            // .bufferProperties = vk::MemoryPropertyFlagBits::eHostVisible 
            //                     | vk::MemoryPropertyFlagBits::eHostCoherent, 
            .bufferProperties = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                                | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .buffer         = &stagingBuffer,
            .bufferMemory   = &stagingBufferMemory,
            .allocationInfo = &allocInfo_staging,
            .vma = vma
        });    
    

    /// -- Map memory to our Temporary Staging Vertex Buffer -- 
    void * data{};
    if(vmaMapMemory(*this->vma, stagingBufferMemory, &data) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate Mesh Staging Vertex Buffer Using VMA!");
    };
    //memcpy(allocInfo_staging.pMappedData, vertices->data(), (static_cast<size_t>(bufferSize))); // TODO: MIGHT BE WRONG
    //memcpy(data, vertices->data(),static_cast<size_t>(bufferSize));
    //memcpy(data, vertices->data(),static_cast<size_t>(bufferSize));
    memcpy(data, vertices->data(),sizeof(Vertex) * vertices->size());
    //vmaUnmapMemory(*this->vma, stagingBufferMemory); 
    vmaUnmapMemory(*this->vma, stagingBufferMemory); 


    if(false)
    {
        /// -- Map memory to our Temporary Staging Vertex Buffer -- 
        // void * data{};                                                                    /// 1. Create pointer to a point in normal memory
        // data = this->device.mapMemory(stagingBufferMemory, 0, bufferSize, vk::MemoryMapFlags());        /// 2. "Map" the vertex buffer memory to that point
        // std::memcpy(data, vertices->data(), (static_cast<size_t>(bufferSize)));         /// 3. Copy memory from vertices Vector to the point        
        // this->device.unmapMemory(stagingBufferMemory);                               /// 4. Unmap the vertex buffer memory        
    }
    

    VmaAllocationInfo allocInfo_deviceOnly;
    /// Create Buffer with TRANSFER_DST_BIT to mark as recipient of transfer data (also VERTEX_BUFFER)
    /// Buffer memory is to be DEVICVE_LOCAL_BIT meaning memory is on the GPU and only accesible by it and not the CPU (HOST)
    vengine_helper::createBuffer(
        {
            .physicalDevice = physicalDevice, 
            .device         = device, 
            .bufferSize     = bufferSize, 
            .bufferUsageFlags = vk::BufferUsageFlagBits::eTransferDst            /// Destination Buffer to be transfered to
                                | vk::BufferUsageFlagBits::eVertexBuffer,    //// This is a Vertex Buffer
            //.bufferProperties = vk::MemoryPropertyFlagBits::eDeviceLocal,        /// This buffer will Only be local to the device (i.e. The GPU)
            .bufferProperties = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
            .buffer         = &vertexBuffer, 
            //.bufferMemory   = &vertexBufferMemory
            .bufferMemory   = &vertexBufferMemory,
            .allocationInfo = &allocInfo_deviceOnly,
            .vma = this->vma

        });

    /// Copy Staging Buffer to Vertex Buffer on GPU
    vengine_helper::copyBuffer(
        this->device, 
        transferQueue, 
        transferCommandPool, 
        stagingBuffer, 
        this->vertexBuffer, 
        bufferSize);

    /// Clean up Staging Buffer stuff
    this->device.destroyBuffer(stagingBuffer);
    //this->device.freeMemory(stagingBufferMemory);
    vmaFreeMemory(*this->vma, stagingBufferMemory);
}

void Mesh::createIndexBuffer(std::vector<uint32_t>* indicies)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    /// Get size of buffer needed for indices
    vk::DeviceSize bufferSize = sizeof(uint32_t) * indicies->size();

    /// Temporary buffer to "Stage" index data before transferring to GPU
    vk::Buffer stagingBuffer{};
    //vk::DeviceMemory stagingBufferMemory{};
    VmaAllocation stagingBufferMemory{};
    VmaAllocationInfo allocInfo_staging;

    vengine_helper::createBuffer(
        {
            .physicalDevice = this->physicalDevice, 
            .device         = this->device, 
            .bufferSize     = bufferSize, 
            .bufferUsageFlags = vk::BufferUsageFlagBits::eTransferSrc, 
            // .bufferProperties = vk::MemoryPropertyFlagBits::eHostVisible 
            //                     | vk::MemoryPropertyFlagBits::eHostCoherent, 
            .bufferProperties = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                                | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .buffer         = &stagingBuffer, 
            //.bufferMemory   = &stagingBufferMemory
            .bufferMemory   = &stagingBufferMemory,
            .allocationInfo = &allocInfo_staging,
            .vma = this->vma
        });

    /// Map Memory to Index Buffer! 
    //void* data{}; 
    //vmaMapMemory(*this->vma, stagingBufferMemory, &data);
    void* data{}; 
    vmaMapMemory(*this->vma, stagingBufferMemory, &data);
    //memcpy(data, indicies->data(), sizeof(bufferSize));
    memcpy(data, indicies->data(), sizeof(uint32_t) * indicies->size());
    vmaUnmapMemory(*this->vma, stagingBufferMemory);
    //vmaUnmapMemory(*this->vma, stagingBufferMemory);
    
    if(false)
    {
        /// Map Memory to Index Buffer! 
        // void * data{};
        // data = this->device.mapMemory(stagingBufferMemory, 0, bufferSize, vk::MemoryMapFlags());
        // memcpy(data, indicies->data(), (size_t)bufferSize); 
        // this->device.unmapMemory( stagingBufferMemory);
    }

    VmaAllocationInfo allocInfo_device;

    /// Create Buffers for INDEX data on GPU access only area
    vengine_helper::createBuffer(
        {
            .physicalDevice = this->physicalDevice, 
            .device         = this->device, 
            .bufferSize     = bufferSize, 
            .bufferUsageFlags = vk::BufferUsageFlagBits::eTransferDst        /// Destination Buffer to be transfered to
                                | vk::BufferUsageFlagBits::eIndexBuffer,     /// This is a Index Buffer, will be used as a Index Buffer
            //.bufferProperties = vk::MemoryPropertyFlagBits::eDeviceLocal,    /// Buffer will be local to the device
            .bufferProperties = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,    /// Buffer will be local to the device
            .buffer         = &this->indexBuffer, 
            //.bufferMemory   = &this->indexBufferMemory
            .bufferMemory   = &this->indexBufferMemory,
            .allocationInfo = &allocInfo_device,
            .vma = this->vma
        });

    vengine_helper::copyBuffer(
        this->device, 
        transferQueue, 
        this->transferCommandPool, 
        stagingBuffer, 
        this->indexBuffer, 
        bufferSize);

    /// Destroy + Release Staging Buffer resources
    this->device.destroyBuffer(stagingBuffer);
    //this->device.freeMemory(stagingBufferMemory);
    vmaFreeMemory(*this->vma,stagingBufferMemory);

}


