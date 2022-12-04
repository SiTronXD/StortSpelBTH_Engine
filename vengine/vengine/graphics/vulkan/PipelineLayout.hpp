#pragma once 
 #include "op_overload.hpp"

#include "../TempPCH.hpp"

class Device;
class ShaderInput;

class PipelineLayout
{
private:
	vk::PipelineLayout pipelineLayout{};

	Device* device;

public:
	PipelineLayout();
	~PipelineLayout();

	void createPipelineLayout(
		Device& device,
		ShaderInput& shaderInput);

	void cleanup();

	inline const vk::PipelineLayout& getVkPipelineLayout() const
	{ return this->pipelineLayout; }
};