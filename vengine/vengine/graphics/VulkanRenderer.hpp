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
#include "vulkan/SubmitArray.hpp"

#include "../application/Window.hpp"
#include "imgui.h"              // Need to be included in header

#include "../resource_management/ResourceManager.hpp"
#include "handlers/LightHandler.hpp"
#include "handlers/PostProcessHandler.hpp"
#include "handlers/ParticleSystemHandler.hpp"
#include "UIRenderer.hpp"
#include "DebugRenderer.hpp"

class Scene;
struct Camera;

using stbi_uc = unsigned char;

class VulkanRenderer 
{
#ifndef VENGINE_NO_PROFILING
    std::vector<TracyVkCtx> tracyContext;
#endif

    const int MAX_FRAMES_IN_FLIGHT = 3;

    ResourceManager* resourceManager;
    UIRenderer* uiRenderer;
    DebugRenderer* debugRenderer;

    Window* window;
    VmaAllocator vma = nullptr;

    int currentFrame = 0; 

    bool windowResized = false;

    PushConstantData pushConstantData{};
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
    RenderPass renderPassSwapchain{};
    RenderPass renderPassImgui{};
    vk::CommandPool commandPool{};
    vk::CommandPool computeCommandPool{};
    CommandBufferArray commandBuffers;
    CommandBufferArray swapchainCommandBuffers;
    CommandBuffer* currentComputeCommandBuffer;
    CommandBuffer* currentShadowMapCommandBuffer;
    CommandBuffer* currentCommandBuffer;
    CommandBuffer* currentSwapchainCommandBuffer;

    // Default pipeline
    UniformBufferID viewProjectionUB;
    UniformBufferID allLightsInfoUB;
    UniformBufferID shadowMapDataUB;
    StorageBufferID lightBufferSB;
    ShaderInput shaderInput;
    Pipeline pipeline;

    // Animations pipeline
    UniformBufferID animViewProjectionUB;
    UniformBufferID animAllLightsInfoUB;
    UniformBufferID animShadowMapDataUB;
    StorageBufferID animLightBufferSB;
    ShaderInput animShaderInput;
    Pipeline animPipeline;

    // Render-to-Swapchain pipeline
    BloomSettingsBufferData bloomSettingsData{};
    UniformBufferID bloomSettingsUB;
    uint32_t hdrRenderTextureDescriptorIndex;
    ShaderInput swapchainShaderInput;
    Pipeline swapchainPipeline;

    LightHandler lightHandler;
    PostProcessHandler postProcessHandler;
    ParticleSystemHandler particleHandler;

    bool hasAnimations;

    // Utilities
    vk::SurfaceFormatKHR  surfaceFormat{};

    // Synchronisation 
    std::vector<vk::Semaphore> imageAvailable;
    std::vector<vk::Semaphore> computeFinished;
    std::vector<vk::Semaphore> shadowMapRenderFinished;
    std::vector<vk::Semaphore> sceneRenderFinished;
    std::vector<std::vector<vk::Semaphore>> downsampleFinished;
    std::vector<std::vector<vk::Semaphore>> upsampleFinished;
    std::vector<vk::Semaphore> swapchainRenderFinished;
    std::vector<vk::Fence> drawFences;

    SubmitArray computeSubmitArray;
    SubmitArray graphicsSubmitArray;

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
    void createCommandPool(
        const uint32_t& queueFamilyIndex,
        vk::CommandPool& outputCommandPool);
    void createSynchronisation();

    // initializations of subsystems
    void initResourceManager();

    // ------- Render functions within render passes -------

    // Compute pass for particles
    void computeParticles();

    // Render pass for shadow map rendering
    std::vector<vk::DeviceSize> bindVertexBufferOffsets;
    std::vector<vk::Buffer> bindVertexBuffers;
    void beginShadowMapRenderPass(
        LightHandler& lightHandler,
        const uint32_t& shadowMapArraySlice);
    void renderShadowMapDefaultMeshes(
        Scene* scene,
        LightHandler& lightHandler);
    void renderShadowMapSkeletalAnimations(
        Scene* scene,
        LightHandler& lightHandler);
    void endShadowMapRenderPass();

    // Render pass for screen rendering
    void beginRenderPass();
    void renderDefaultMeshes(Scene* scene);
    void renderSkeletalAnimations(Scene* scene);
    void renderParticles(Scene* scene);
    void endRenderPass();

    // Render pass for bloom downsampling
    void beginBloomDownUpsampleRenderPass(
        const RenderPass& renderPass,
        CommandBuffer& commandBuffer,
        const uint32_t& writeMipIndex,
        bool isUpsampling);
    void renderBloomDownUpsample(
        CommandBuffer& commandBuffer,
        ShaderInput& shaderInput,
        const Pipeline& pipeline,
        const uint32_t& readMipIndex);
    void endBloomDownUpsampleRenderPass(
        CommandBuffer& commandBuffer);

    // Render pass for swapchain image rendering
    void beginSwapchainRenderPass(
        const uint32_t& imageIndex);
    void renderToSwapchainImage();
    void renderUI();
    void renderDebugElements();
    void endSwapchainRenderPass();

    // Render pass for imgui rendering
    void beginRenderpassImgui(
        const uint32_t& imageIndex);
    void renderImgui();
    void endRenderpassImgui();

    
    void recordCommandBuffers(
        Scene* scene, 
        Camera* camera,
        uint32_t imageIndex);

    const Material& getAppropriateMaterial(
        const MeshComponent& meshComponent,
        const std::vector<SubmeshData>& submeshes,
        const uint32_t& submeshIndex);
    Material& getAppropriateMaterial(
        MeshComponent& meshComponent,
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

    // VMA 
    void generateVmaDump();

    bool& getWindowResized() { return this->windowResized; }
};
