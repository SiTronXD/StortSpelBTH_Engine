#pragma once

#include "PipelineLayout.h"
#include "RenderPass.h"

class Pipeline
{
private:
	VkPipeline pipeline;

	Renderer& renderer;

public:
	Pipeline(Renderer& renderer);
	~Pipeline();

	void createGraphicsPipeline(
		PipelineLayout& pipelineLayout,
		const RenderPass& renderPass,
		bool wireframe = false);

	void cleanup();

	inline const VkPipeline& getVkPipeline() const { return this->pipeline; }
};