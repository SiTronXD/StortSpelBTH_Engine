#pragma once

#include "Buffer.h"

class IndexBuffer : public Buffer
{
private:
	VkDeviceSize bufferSize;

	bool isBufferDynamic;

	void createStaticGpuIndexBuffer(const std::vector<uint32_t>& indices);
	void createDynamicCpuIndexBuffer(const std::vector<uint32_t>& indices);

public:
	IndexBuffer(Renderer& renderer);
	~IndexBuffer();

	void createIndexBuffer(
		const std::vector<uint32_t>& indices,
		bool cpuWriteable = false);

	void updateIndexBuffer(const std::vector<uint32_t>& indices, uint32_t bufferIndex);

	inline const bool& getIsBufferDynamic() const { return this->isBufferDynamic; }
};