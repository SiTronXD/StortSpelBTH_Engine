#pragma once

#define NOMINMAX

#include "vulkan/VulkanInstance.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/Swapchain.hpp"
#include "vulkan/QueueFamilies.hpp"
#include "vulkan/Pipeline.hpp"
#include "vulkan/UniformBuffer.hpp"
#include "vulkan/CommandBufferArray.hpp"

#include "../application/Window.hpp"
#include "imgui.h"              // Need to be included in header

#include "../resource_management/ResourceManager.hpp"
#include "UIRenderer.hpp"
#include "DebugRenderer.hpp"
#include "vulkan/UniformBufferStructs.hpp"

class Scene;
class Camera;

struct LightBufferData
{
    glm::vec4 position;
    glm::vec4 color;
    glm::vec4 padding0;
    glm::vec4 padding1;
};

#include <functional>
using stbi_uc = unsigned char;
class VulkanRenderer 
{
#ifndef VENGINE_NO_PROFILING
    std::vector<TracyVkCtx> tracyContext;
#endif
    const int MAX_FRAMES_IN_FLIGHT = 3;

    const uint32_t MAX_NUM_LIGHTS = 1;

    ResourceManager* resourceManager;
    UIRenderer* uiRenderer;
    DebugRenderer* debugRenderer;

    Window* window;
    VmaAllocator vma = nullptr;

    int currentFrame = 0; 

    bool windowResized = false;

    UboViewProjection uboViewProjection{};

    // Vulkan Components
    // - Main
    VulkanInstance instance;
    vk::DispatchLoaderDynamic dynamicDispatch;
    vk::DebugUtilsMessengerEXT debugMessenger{}; // used to handle callback from errors based on the validation Layer (??)
    VkDebugReportCallbackEXT debugReport{};
    
    PhysicalDevice physicalDevice;
    Device device;
    QueueFamilies queueFamilies;

    vk::SurfaceKHR surface{};            //Images will be displayed through a surface, which GLFW will read from
    
    Swapchain swapchain;
    
    vk::RenderPass renderPassBase{};
    vk::RenderPass renderPassImgui{};
    vk::CommandPool commandPool{};
    CommandBufferArray commandBuffers;

    std::vector<LightBufferData> lightBuffer;

    // Default pipeline
    UniformBufferID viewProjectionUB;
    SamplerID sampler;
    StorageBufferID lightBufferSB;
    ShaderInput shaderInput;
    Pipeline pipeline;

    // Animations pipeline
	bool hasAnimations;
    UniformBufferID animViewProjectionUB;
    SamplerID animSampler;
    StorageBufferID animLightBufferID;
    ShaderInput animShaderInput;
    Pipeline animPipeline;

    // - Utilities
    vk::SurfaceFormatKHR  surfaceFormat{};

    // - Synchronisation 
    std::vector<vk::Semaphore> imageAvailable;
    std::vector<vk::Semaphore> renderFinished;
    std::vector<vk::Fence>     drawFences;
    
    char* tracyImage{};
    // - Debug Utilities
    // - - ImGui
    void initImgui();
    void createFramebufferImgui();
    void cleanupFramebufferImgui();
    vk::DescriptorPool descriptorPoolImgui;
    std::vector<vk::Framebuffer> frameBuffersImgui;


    // - - Tracy
#ifndef VENGINE_NO_PROFILING    
    void initTracy();
#endif

private:

    // Vulkan Functions
    // - Create functions
    void setupDebugMessenger();
    void createSurface();
    void recreateSwapchain(Camera* camera);
    void createRenderPassBase();
    void createRenderPassImgui();
    void createCommandPool();   //TODO: Deprecate! 
    void createSynchronisation();

    // initializations of subsystems
    void initResourceManager();

    // Cleanup 
    void cleanupRenderPassImgui();
    void cleanupRenderPassBase();

    // Newer Create functions! 
    void createCommandPool(vk::CommandPool& commandPool, vk::CommandPoolCreateFlags flags, std::string&& name);
    void createFramebuffer(vk::Framebuffer& frameBuffer,std::vector<vk::ImageView>& attachments,vk::RenderPass& renderPass, vk::Extent2D& extent, std::string&& name);

    void updateUboProjection();
    void updateUboView(glm::vec3 eye, glm::vec3 center, glm::vec3 up = glm::vec3(0.F,1.F,0.F));
    void updateLightBuffer(Scene* scene);

    // - Record Functions
    void recordCommandBuffer(Scene* scene, uint32_t imageIndex);    // Using renderpass

    inline vk::Device& getVkDevice() { return this->device.getVkDevice(); }

private: 
    // Clients Privates 
    std::function<void()> gameLoopFunction;

public:
    VulkanRenderer();
    ~VulkanRenderer()   = default;    
    VulkanRenderer(const VulkanRenderer &ref)              = delete;
    VulkanRenderer(VulkanRenderer &&ref)                   = delete;
    VulkanRenderer& operator=(const VulkanRenderer &ref)   = delete;
    VulkanRenderer& operator=(VulkanRenderer &&ref)        = delete;

    int init(
        Window* window, 
        std::string&& windowName, 
        ResourceManager* resourceMan,
        UIRenderer* uiRenderer,
        DebugRenderer* debugRenderer);
    void draw(Scene* scene);

    void initForScene(Scene* scene);
    
    void cleanup();

    // - - VMA 
    void generateVmaDump();

    bool& getWindowResized() { return this->windowResized; }
};
