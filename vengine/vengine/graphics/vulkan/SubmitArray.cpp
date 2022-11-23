#include "pch.h"
#include "SubmitArray.hpp"

void SubmitArray::setMaxNumSubmits(const uint32_t& maxNumSubmits)
{
    this->maxNumSubmits = maxNumSubmits;

    this->commandBufferInfos.resize(this->maxNumSubmits);
    this->waitSemaphores.resize(this->maxNumSubmits);
    this->signalSemaphores.resize(this->maxNumSubmits);
    this->submitInfos.resize(this->maxNumSubmits);
}

void SubmitArray::reset()
{
    this->currentSubmitIndex = 0;
}

void SubmitArray::setSubmitInfo(
    CommandBuffer& commandBuffer,
    std::array<vk::SemaphoreSubmitInfo, 4>& waitSemaphores,
    const uint32_t& numWaitSemaphores,
    vk::Semaphore& signalSemaphore)
{
    // New submit info
    vk::SubmitInfo2& submitInfo = this->submitInfos[this->currentSubmitIndex];

    // Command buffer
    this->commandBufferInfos[this->currentSubmitIndex].setCommandBuffer(commandBuffer.getVkCommandBuffer());
    submitInfo.setCommandBufferInfoCount(uint32_t(1));
    submitInfo.setPCommandBufferInfos(&this->commandBufferInfos[this->currentSubmitIndex]); // Pointer to the command buffer to execute

    // Wait semaphores
    this->waitSemaphores[this->currentSubmitIndex] = waitSemaphores;
    submitInfo.setWaitSemaphoreInfoCount(numWaitSemaphores);
    submitInfo.setPWaitSemaphoreInfos(this->waitSemaphores[this->currentSubmitIndex].data());       // Pointer to the semaphore to wait on.
    
    // Signal semaphores
    this->signalSemaphores[this->currentSubmitIndex].setSemaphore(signalSemaphore);
    submitInfo.setSignalSemaphoreInfoCount(uint32_t(1));
    submitInfo.setPSignalSemaphoreInfos(&this->signalSemaphores[this->currentSubmitIndex]);   // Semaphore that will be signaled when CommandBuffer is finished

    // Next submit index
    this->currentSubmitIndex++;
}