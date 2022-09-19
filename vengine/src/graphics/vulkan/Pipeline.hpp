#pragma once

#include "../TempPCH.hpp"

class Device;

class Pipeline
{
private:
	vk::Pipeline pipeline{};

	Device* device;

public:
	Pipeline();
	~Pipeline();

	void createPipeline(Device& device);

	void cleanup();
};