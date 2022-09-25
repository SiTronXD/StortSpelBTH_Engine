#pragma once

#define NOMINMAX

#include "vulkan/VulkanInstance.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/Swapchain.hpp"
#include "vulkan/QueueFamilies.hpp"
#include "vulkan/PipelineLayout.hpp"
#include "vulkan/Pipeline.hpp"
#include "vulkan/UniformBuffer.hpp"

#include "../application/Window.hpp"
#include "imgui.h"              // Need to be included in header

#include "../application/ResourceManager.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/fwd.hpp"

class Scene;
class Camera;

#include <functional>
using stbi_uc = unsigned char;
class VulkanRenderer 
{
#ifndef VENGINE_NO_PROFILING
    std::vector<TracyVkCtx> tracyContext;
#endif
    const int MAX_FRAMES_IN_FLIGHT = 3;
    friend class TextureLoader;         /// TODO: REMOVE , Just to give TextureLoader access to SamplerDescriptor...
    ResourceManager* resourceMan;    

    Window* window;
    VmaAllocator vma = nullptr;

    int currentFrame = 0; 

    bool windowResized = false;

    // Scene Settings
    struct UboViewProjection 
    {
        glm::mat4 projection;   // How the Camera Views the world (Ortographic or Perspective)
        glm::mat4 view;         // Where our Camera is viewing from and the direction it is viewing        
    } uboViewProjection{};

    //Vulkan Components
    // - Main
    
    VulkanInstance instance;
    vk::DispatchLoaderDynamic dynamicDispatch;
    vk::DebugUtilsMessengerEXT debugMessenger{}; // used to handle callback from errors based on the validation Layer (??)
    VkDebugReportCallbackEXT debugReport{};
    
    PhysicalDevice physicalDevice;
    Device device;
    QueueFamilies queueFamilies;

    vk::SurfaceKHR        surface{};            //Images will be displayed through a surface, which GLFW will read from
    
    Swapchain swapchain;
    std::vector<vk::CommandBuffer> commandBuffers;
    
    vk::Sampler       textureSampler{}; // Sampler used to sample images in order to present (??)

    // - Descriptors
    vk::DescriptorSetLayout samplerDescriptorSetLayout{};
    vk::DescriptorSetLayout descriptorSetLayout{};
    vk::PushConstantRange pushConstantRange{};      

    std::vector<vk::DescriptorSet> descriptorSets;        // To be used with our View and Projection matrices
    std::vector<vk::DescriptorSet> samplerDescriptorSets; // To be used for our texture Samplers! (need one per Texture)
                                                        // NOTE; There will NOT be one samplerDescriptionSet per image!... It will be One per Texture!

    UniformBuffer viewProjectionUB;

    // - Assets    

    std::vector<vk::Image>        textureImages;
    std::vector<VmaAllocation> textureImageMemory;
    std::vector<vk::ImageView>    textureImageViews;

    // - Pipeline
    PipelineLayout pipelineLayout;
    Pipeline pipeline;
    vk::PipelineCache graphics_pipelineCache = nullptr;
    vk::RenderPass renderPassBase{};
    vk::RenderPass renderPassImgui{};

    // - Pools
    vk::CommandPool    graphicsCommandPool{};           // Command pool that only is used for graphics command...
    vk::DescriptorPool descriptorPool{};
    vk::DescriptorPool samplerDescriptorPool{};

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
    void createDescriptorSetLayout();
    void createPushConstantRange();
    void createCommandPool();   //TODO: Deprecate! 
    void createCommandBuffers(); //TODO: Deprecate!  //Allocate Command Buffers from Command pool...
    void createSynchronisation();
    void createTextureSampler();

    void createDescriptorPool();
    void createDescriptorSets();
    void allocateDescriptorSets();

    // initializations of subsystems
    void initResourceManager();

    // Cleanup 
    void cleanupRenderPassImgui();
    void cleanupRenderPassBase();

    // Newer Create functions! 
    void createCommandPool(vk::CommandPool& commandPool, vk::CommandPoolCreateFlags flags, std::string&& name);
    void createCommandBuffer(vk::CommandBuffer& commandBuffer,vk::CommandPool& commandPool, std::string&& name);
    void createFramebuffer(vk::Framebuffer& frameBuffer,std::vector<vk::ImageView>& attachments,vk::RenderPass& renderPass, vk::Extent2D& extent, std::string&& name);

    void updateUboProjection();
    void updateUboView(glm::vec3 eye, glm::vec3 center, glm::vec3 up = glm::vec3(0.F,1.F,0.F));

    // - Record Functions
    void recordRenderPassCommandsBase(Scene* scene, uint32_t imageIndex);    // Using renderpass

    // -- Create Functions    
    [[nodiscard]] vk::ShaderModule createShaderModule(const std::vector<char> &code);

    int createTextureImage(const std::string &filename);
    int createTexture(const std::string &filename);
    int createSamplerDescriptor(vk::ImageView textureImage);

    // -- Loader Functions
    static stbi_uc* loadTextuerFile(const std::string &filename, int* width, int* height, vk::DeviceSize* imageSize );

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

    int  init(Window* window, std::string&& windowName, ResourceManager* resourceMan);
    int  createModel(const std::string &modelFile);
    void updateModel(int modelIndex, glm::mat4 newModel);
    void draw(Scene* scene);

    void initMeshes(Scene* scene);
    
    void cleanup();

    // - - VMA 
    void generateVmaDump();

    bool& getWindowResized() { return this->windowResized; }
};
