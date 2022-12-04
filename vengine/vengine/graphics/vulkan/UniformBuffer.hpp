#pragma once 
 #include "op_overload.hpp"

#include "../Buffer.hpp"

class UniformBuffer : public Buffer
{
private:
public:
	void createUniformBuffer(
		Device& device,
		VmaAllocator& vma,
		const size_t& bufferSize, 
		const uint32_t& framesInFlight);
};