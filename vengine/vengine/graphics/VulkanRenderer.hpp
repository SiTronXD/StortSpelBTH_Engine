#pragma once

#define NOMINMAX

#include <functional>

#include "vulkan/VulkanInstance.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/Swapchain.hpp"
#include "vulkan/QueueFamilies.hpp"
#include "vulkan/RenderPass.hpp"
#include "vulkan/Pipeline.hpp"
#include "vulkan/UniformBuffer.hpp"
#include "vulkan/CommandBufferArray.hpp"
#include "vulkan/FramebufferArray.hpp"

#include "../application/Window.hpp"
#include "imgui.h"              // Need to be included in header

#include "../resource_management/ResourceManager.hpp"
#include "UIRenderer.hpp"
#include "DebugRenderer.hpp"
#include "vulkan/UniformBufferStructs.hpp"

class Scene;
class Camera;

using stbi_uc = unsigned char;

class VulkanRenderer 
{
#ifndef VENGINE_NO_PROFILING
    std::vector<TracyVkCtx> tracyContext;
#endif

    const int MAX_FRAMES_IN_FLIGHT = 3;
    const uint32_t MAX_NUM_LIGHTS = 16;

    struct PushConstantData
    {
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        glm::vec4 tintColor = glm::vec4(0.0f); // vec4(R, G, B, lerp alpha)
    } pushConstantData{};

    ResourceManager* resourceManager;
    UIRenderer* uiRenderer;
    DebugRenderer* debugRenderer;

    Window* window;
    VmaAllocator vma = nullptr;

    int currentFrame = 0; 

    bool windowResized = false;

    CameraBufferData cameraDataUBO{};

    // Vulkan Components
    VulkanInstance instance;
    vk::DispatchLoaderDynamic dynamicDispatch;
    vk::DebugUtilsMessengerEXT debugMessenger{}; // Used to handle callback from errors based on the validation layer
    VkDebugReportCallbackEXT debugReport{};
    
    PhysicalDevice physicalDevice;
    Device device;
    QueueFamilies queueFamilies;

    vk::SurfaceKHR surface{};            // Images will be displayed through a surface, which SDL will read from
    
    Swapchain swapchain;
    
    RenderPass renderPassBase{};
    RenderPass renderPassImgui{};
    vk::CommandPool commandPool{};
    CommandBufferArray commandBuffers;
    CommandBuffer* currentShadowMapCommandBuffer;
    CommandBuffer* currentCommandBuffer;

    std::vector<LightBufferData> lightBuffer;




    // Shadow map stuff, TODO: place this somewhere else later
    Texture shadowMapTexture;
    RenderPass shadowMapRenderPass;
    FramebufferArray shadowMapFramebuffer;
    UniformBufferID shadowMapViewProjectionUB;
    ShaderInput shadowMapShaderInput;
    Pipeline shadowMapPipeline;
    CommandBufferArray shadowMapCommandBuffers;
    vk::Extent2D shadowMapExtent;




    // Default pipeline
    UniformBufferID viewProjectionUB;
    UniformBufferID allLightsInfoUB;
    StorageBufferID lightBufferSB;
    ShaderInput shaderInput;
    Pipeline pipeline;

    // Animations pipeline
	bool hasAnimations;
    UniformBufferID animViewProjectionUB;
    UniformBufferID animAllLightsInfoUB;
    StorageBufferID animLightBufferSB;
    ShaderInput animShaderInput;
    Pipeline animPipeline;

    // Utilities
    vk::SurfaceFormatKHR  surfaceFormat{};

    // Synchronisation 
    std::vector<vk::Semaphore> imageAvailable;
    std::vector<vk::Semaphore> shadowMapRenderFinished;
    std::vector<vk::Semaphore> renderFinished;
    std::vector<vk::Fence> drawFences;
    
    char* tracyImage{};

    // ImGui
    void initImgui();
    vk::DescriptorPool descriptorPoolImgui;
    FramebufferArray frameBuffersImgui;

    // Tracy
#ifndef VENGINE_NO_PROFILING    
    void initTracy();
#endif

private:
    // Create functions
    void setupDebugMessenger();
    void createSurface();
    void windowResize(Camera* camera);
    void createCommandPool();   //TODO: Deprecate! 
    void createSynchronisation();
    void createCommandPool(
        vk::CommandPool& commandPool,
        vk::CommandPoolCreateFlags flags,
        std::string&& name);

    // initializations of subsystems
    void initResourceManager();

    void updateLightBuffer(Scene* scene);

    // ------- Render functions within render passes -------

    // Render pass for shadow map rendering
    void beginShadowMapRenderPass(const uint32_t& imageIndex);
    void renderShadowMapDefaultMeshes(Scene* scene);
    void endShadowMapRenderPass();

    // Render pass for screen rendering
    void beginRenderpass(
        const uint32_t& imageIndex);
    void renderDefaultMeshes(Scene* scene);
    void renderSkeletalAnimations(Scene* scene);
    void renderUI();
    void renderDebugElements();
    void endRenderpass();

    // Render pass for imgui rendering
    void beginRenderpassImgui(
        const uint32_t& imageIndex);
    void renderImgui();
    void endRenderpassImgui();

    // Record Functions
    void recordCommandBuffer(Scene* scene, uint32_t imageIndex);    // Using renderpass

    const Material& getAppropriateMaterial(
        const MeshComponent& meshComponent,
        const std::vector<SubmeshData>& submeshes,
        const uint32_t& submeshIndex);

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
