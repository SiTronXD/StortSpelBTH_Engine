#pragma once 
 #include "op_overload.hpp"

#include "../Buffer.hpp"

class StorageBuffer : public Buffer
{
private:

public:
	void createStorageBuffer(
        Device& device,
        VmaAllocator& vma,
        const size_t& bufferSize,
        uint32_t framesInFlight,
        const bool& gpuOnly,
        void* initialData,
        vk::Queue* transferQueue,
        vk::CommandPool* transferCommandPool);
};