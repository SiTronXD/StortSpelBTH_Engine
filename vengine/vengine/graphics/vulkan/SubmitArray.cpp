#include "pch.h"
#include "SubmitArray.hpp"

void SubmitArray::setNumSubmits(const uint32_t& numSubmits)
{
    this->numSubmits = numSubmits;

    this->commandBufferInfos.resize(this->numSubmits);
    this->waitSemaphores.resize(this->numSubmits);
    this->signalSemaphores.resize(this->numSubmits);
    this->submitInfos.resize(this->numSubmits);
}

void SubmitArray::setSubmitInfo(
    CommandBuffer& commandBuffer,
    std::array<vk::SemaphoreSubmitInfo, 4>& waitSemaphores,
    const uint32_t& numWaitSemaphores,
    vk::Semaphore& signalSemaphore,
    const uint32_t& submitIndex)
{
    // New submit info
    vk::SubmitInfo2& submitInfo = this->submitInfos[submitIndex];

    // Command buffer
    this->commandBufferInfos[submitIndex].setCommandBuffer(commandBuffer.getVkCommandBuffer());
    submitInfo.setCommandBufferInfoCount(uint32_t(1));
    submitInfo.setPCommandBufferInfos(&this->commandBufferInfos[submitIndex]); // Pointer to the command buffer to execute

    // Wait semaphores
    this->waitSemaphores[submitIndex] = waitSemaphores;
    submitInfo.setWaitSemaphoreInfoCount(numWaitSemaphores);
    submitInfo.setPWaitSemaphoreInfos(this->waitSemaphores[submitIndex].data());       // Pointer to the semaphore to wait on.
    
    // Signal semaphores
    this->signalSemaphores[submitIndex].setSemaphore(signalSemaphore);
    submitInfo.setSignalSemaphoreInfoCount(uint32_t(1));
    submitInfo.setPSignalSemaphoreInfos(&this->signalSemaphores[submitIndex]);   // Semaphore that will be signaled when CommandBuffer is finished

}