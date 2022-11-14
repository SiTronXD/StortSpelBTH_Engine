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
        const uint32_t& framesInFlight);
};