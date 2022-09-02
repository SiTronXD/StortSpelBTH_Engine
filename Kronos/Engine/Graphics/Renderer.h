#pragma once

#include <optional>

#include "../Dev/Log.h"
#include "../Application/Window.h"
#include "Vulkan/Instance.h"
#include "Vulkan/DebugMessenger.h"
#include "Vulkan/Surface.h"
#include "Vulkan/PhysicalDevice.h"
#include "Vulkan/Device.h"
#include "Vulkan/RenderPass.h"
#include "Vulkan/DescriptorSetLayout.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/CommandBufferArray.h"
#include "Vulkan/DescriptorPool.h"
#include "Vulkan/DescriptorSetArray.h"
#include "Texture.h"
#include "Swapchain.h"
#include "Camera.h"
#include "Mesh.h"

struct UniformBufferObject
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

class Renderer
{
private:
	const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

	Instance instance;
	DebugMessenger debugMessenger;
	Surface surface;
	PhysicalDevice physicalDevice;
	Device device;
	QueueFamilies queueFamilies;
	Swapchain swapchain;

	RenderPass renderPass;
	DescriptorSetLayout descriptorSetLayout;
	PipelineLayout graphicsPipelineLayout;
	Pipeline graphicsPipeline;

	CommandPool commandPool;
	CommandPool singleTimeCommandPool;
	CommandBufferArray commandBuffers;

	DescriptorPool descriptorPool;
	DescriptorSetArray descriptorSets;

	DescriptorPool imguiDescriptorPool;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;

	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;

	Window* window;

	uint32_t currentFrameIndex = 0;

	Texture texture;

	void initVulkan();

	void createUniformBuffers();
	void createSyncObjects();

	void updateUniformBuffer(uint32_t currentImage, Camera& camera);

	void recordCommandBuffer(uint32_t imageIndex, Mesh& mesh);

public:
	bool framebufferResized = false;

	Renderer();
	~Renderer();

	void init();
	void setWindow(Window& window);
	void startCleanup();
	void cleanup();

	void drawFrame(Camera& camera, Mesh& mesh);

	void setToWireframe(bool wireframe);

	// Vulkan
	inline VkInstance& getVkInstance() { return this->instance.getVkInstance(); }
	inline VkPhysicalDevice& getVkPhysicalDevice() { return this->physicalDevice.getVkPhysicalDevice(); }
	inline VkDevice& getVkDevice() { return this->device.getVkDevice(); }

	inline Surface& getSurface() { return this->surface; }
	inline CommandPool& getCommandPool() { return this->commandPool; }
	inline CommandPool& getSingleTimeCommandPool() { return this->singleTimeCommandPool; }
	inline QueueFamilies& getQueueFamilies() { return this->queueFamilies; }
	inline RenderPass& getRenderPass() { return this->renderPass; }
	inline Swapchain& getSwapchain() { return this->swapchain; }
	inline Window& getWindow() { return *this->window; }
	inline DescriptorPool& getImguiDescriptorPool() { return this->imguiDescriptorPool; }

	inline float getSwapchainAspectRatio() 
		{ return (float) this->swapchain.getWidth() / this->swapchain.getHeight(); }
	inline const uint32_t& getMaxFramesInFlight() const { return this->MAX_FRAMES_IN_FLIGHT; }
	inline const uint32_t& getCurrentFrameIndex() const { return this->currentFrameIndex; }
};