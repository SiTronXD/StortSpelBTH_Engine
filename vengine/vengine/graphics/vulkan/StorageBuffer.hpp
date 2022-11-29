#pragma once

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
        const bool& gpuOnly);
};