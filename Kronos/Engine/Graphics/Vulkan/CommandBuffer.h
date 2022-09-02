#pragma once

#include <vector>

#include "CommandPool.h"
#include "Pipeline.h"
#include "DescriptorSet.h"

class VertexBuffer;
class IndexBuffer;

class CommandBuffer
{
private:
	VkCommandBuffer commandBuffer;

public:
	CommandBuffer();
	~CommandBuffer();

	void resetAndBegin();

	void beginRenderPass(const VkRenderPassBeginInfo& renderPassInfo);
	void bindPipeline(const Pipeline& pipeline);
	void setViewport(const VkViewport& viewport);
	void setScissor(const VkRect2D& scissor);
	void bindVertexBuffer(VertexBuffer& vertexBuffer);
	void bindIndexBuffer(IndexBuffer& indexBuffer, uint32_t frameIndex);
	void bindDescriptorSet(
		const PipelineLayout& pipelineLayout,
		const DescriptorSet& descriptorSet);
	void drawIndexed(size_t numIndices);

	void endRenderPass();
	void end();

	void setVkCommandBuffer(const VkCommandBuffer& commandBuffer);

	inline VkCommandBuffer& getVkCommandBuffer() { return this->commandBuffer; }


	static VkCommandBuffer beginSingleTimeCommands(Renderer& renderer);
	static void endSingleTimeCommands(
		Renderer& renderer,
		VkCommandBuffer commandBuffer);
};