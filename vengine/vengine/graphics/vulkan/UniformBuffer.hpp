#pragma once

#include "../Buffer.hpp"

class UniformBuffer : public Buffer
{
private:
public:
	UniformBuffer();
	~UniformBuffer();

	void createUniformBuffer(
		Device& device,
		VmaAllocator& vma,
		const size_t& bufferSize, 
		const uint32_t& framesInFlight);

	void update(
		void* data,
		const uint32_t& currentFrame);

	void cleanup();
};