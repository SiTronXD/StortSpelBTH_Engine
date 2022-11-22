#pragma once

#include <vector>
#include <array>
#include <vulkan/vulkan.hpp>
#include "CommandBuffer.hpp"

class SubmitArray
{
private:
	std::vector<vk::CommandBufferSubmitInfo> commandBufferInfos;
	std::vector<std::array<vk::SemaphoreSubmitInfo, 4>> waitSemaphores;
	std::vector<vk::SemaphoreSubmitInfo> signalSemaphores;
	std::vector<vk::SubmitInfo2> submitInfos;

	uint32_t numSubmits;

public:
	void setNumSubmits(const uint32_t& numSubmits);

	void setSubmitInfo(
		CommandBuffer& commandBuffer,
		std::array<vk::SemaphoreSubmitInfo, 4>& waitSemaphores,
		const uint32_t& numWaitSemaphores,
		vk::Semaphore& signalSemaphore,
		const uint32_t& submitIndex);

	inline const uint32_t& getNumSubmits() const { return this->numSubmits; }
	inline const std::vector<vk::SubmitInfo2>& getSubmitInfos() const { return this->submitInfos; }
};