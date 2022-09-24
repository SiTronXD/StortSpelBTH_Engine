#pragma once

#define NOMINMAX

#include "VulkanInstance.hpp"
#include "Device.hpp"
#include "Window.hpp"
#include "Swapchain.hpp"
#include "QueueFamilies.hpp"

#include "Window.hpp"
#include "imgui.h"              // Need to be included in header

#include "glm/gtc/matrix_transform.hpp"
#include "glm/fwd.hpp"

class Scene;
class Camera;

#include <functional>
using stbi_uc = unsigned char;
class VulkanRenderer {
#ifndef VENGINE_NO_PROFILING
    std::vector<TracyVkCtx> tracyContext;
#endif
    friend class TextureLoader;         /// TODO: REMOVE , Just to give TextureLoader access to SamplerDescriptor...
    Window* window;
    VmaAllocator vma = nullptr;

    int currentFrame = 0; 

    bool windowResized = false;
    /// Scene Objects
    //std::vector<Mesh> meshes;

    /// Scene Settings
    struct UboViewProjection 
    {
        glm::mat4 projection;   /// How the Camera Views the world (Ortographic or Perspective)
        glm::mat4 view;         /// Where our Camera is viewing from and the direction it is viewing        
    } uboViewProjection{};

    ///Vulkan Components
    /// - Main
    
    VulkanInstance instance;
    vk::DispatchLoaderDynamic dynamicDispatch;
    vk::DebugUtilsMessengerEXT debugMessenger{}; /// used to handle callback from errors based on the validation Layer (??)
    VkDebugReportCallbackEXT debugReport{};
    
    PhysicalDevice physicalDevice;
    Device device;
    QueueFamilies queueFamilies;

    vk::SurfaceKHR        surface{};            ///Images will be displayed through a surface, which GLFW will read from
    
    Swapchain swapchain;
    std::vector<vk::CommandBuffer> commandBuffers;
    
    vk::Sampler       textureSampler{}; /// Sampler used to sample images in order to present (??)

    /// - Descriptors
    vk::DescriptorSetLayout samplerDescriptorSetLayout{};
    vk::DescriptorSetLayout descriptorSetLayout{};
    vk::DescriptorSetLayout inputSetLayout{};
    vk::PushConstantRange pushConstantRange{};      

    std::vector<vk::DescriptorSet> descriptorSets;        /// To be used with our View and Projection matrices
    std::vector<vk::DescriptorSet> inputDescriptorSets;   /// To be used by Subpasses; input attachment descriptors...
    std::vector<vk::DescriptorSet> samplerDescriptorSets; /// To be used for our texture Samplers! (need one per Texture)
                                                        //// NOTE; There will NOT be one samplerDescriptionSet per image!... It will be One per Texture!

    std::vector<vk::Buffer> viewProjection_uniformBuffer;
    std::vector<VmaAllocation> viewProjection_uniformBufferMemory;
    std::vector<VmaAllocationInfo> viewProjection_uniformBufferMemory_info;

    // - Assets    

    std::vector<vk::Image>        textureImages;
    //std::vector<vk::DeviceMemory> textureImageMemory;
    std::vector<VmaAllocation> textureImageMemory;
    std::vector<vk::ImageView>    textureImageViews;

    /// - Pipeline
    vk::Pipeline       graphicsPipeline{};
    vk::PipelineCache  graphics_pipelineCache = nullptr;
    vk::PipelineLayout pipelineLayout{};
    vk::RenderPass     renderPass_base{};
    vk::RenderPass     renderPass_imgui{};

    vk::Pipeline       secondGraphicsPipeline{};
    vk::PipelineLayout secondPipelineLayout{}; 

    /// - Pools
    vk::CommandPool    graphicsCommandPool{};           /// Command pool that only is used for graphics command...
    vk::DescriptorPool descriptorPool{};
    vk::DescriptorPool inputDescriptorPool{};           /// used by Subpasses for input Attachments
    vk::DescriptorPool samplerDescriptorPool{};

    /// - Utilities
    vk::SurfaceFormatKHR  surfaceFormat{};

    /// - Synchronisation 
    std::vector<vk::Semaphore> imageAvailable;
    std::vector<vk::Semaphore> renderFinished;
    std::vector<vk::Fence>     drawFences;
    
    char* tracyImage{};
    /// - Debug Utilities
    /// - - ImGui
    void initImgui();
    void createFramebuffer_imgui();
    void cleanupFramebuffer_imgui();
    vk::DescriptorPool descriptorPool_imgui;
    std::vector<vk::Framebuffer> frameBuffers_imgui;


    /// - - Tracy
#ifndef VENGINE_NO_PROFILING    
    void initTracy();
    //// - - Tracy  Callbacks
    bool TracyThumbnail_bool = false;
#endif
public:
#ifndef VENGINE_NO_PROFILING    
    /// TODO: this should not be visible to client...
    // void toggleTracyThumbnail(bool toggle){
    //     TracyThumbnail_bool = toggle;
    // };
#endif

private:

    ///Vulkan Functions
    /// - Create functions
    void setupDebugMessenger();
    void createSurface();
    void reCreateSwapChain(Camera* camera);    
    void createRenderPass_Base();
    void createRenderPass_Imgui();
    void createDescriptorSetLayout();
    void createPushConstantRange();
    void createGraphicsPipeline_Base();
    void createGraphicsPipeline_Imgui();
    void createCommandPool();   //TODO: Deprecate! 
    void createCommandBuffers(); //TODO: Deprecate!  //Allocate Command Buffers from Command pool...
    void createSynchronisation();
    void createTextureSampler();

    void createUniformBuffers();
    void createDescriptorPool();
    void createDescriptorSets();
    void allocateDescriptorSets();
    void createInputDescriptorSets();

    /// ResourceManager
    void initResourceManager();

    // Cleanup 
    void cleanupRenderBass_Imgui();
    void cleanupRenderBass_Base(); //TODO

    /// Newer Create functions! 
    void createCommandPool(vk::CommandPool& commandPool, vk::CommandPoolCreateFlags flags, std::string&& name);
    void createCommandBuffer(vk::CommandBuffer& commandBuffer,vk::CommandPool& commandPool, std::string&& name);
    void createFrameBuffer(vk::Framebuffer& frameBuffer,std::vector<vk::ImageView>& attachments,vk::RenderPass& renderPass, vk::Extent2D& extent, std::string&& name);

    void updateUniformBuffers(uint32_t imageIndex);
    void updateUBO_camera_Projection();
    void updateUBO_camera_view(glm::vec3 eye, glm::vec3 center, glm::vec3 up = glm::vec3(0.F,1.F,0.F));

    /// - Record Functions
    void recordRenderPassCommands_Base(Scene* scene, uint32_t currentImageIndex);    // Using renderpass

    /// -- Create Functions    
    [[nodiscard]] vk::ShaderModule createShaderModule(const std::vector<char> &code);


    inline vk::Device& getVkDevice() { return this->device.getVkDevice(); }

private: 
    /// Clients Privates 
    std::function<void()> gameLoopFunction;

public:
    VulkanRenderer();
    ~VulkanRenderer()   = default;    
    VulkanRenderer(const VulkanRenderer &ref)              = delete;
    VulkanRenderer(VulkanRenderer &&ref)                   = delete;
    VulkanRenderer& operator=(const VulkanRenderer &ref)   = delete;
    VulkanRenderer& operator=(VulkanRenderer &&ref)        = delete;

    int  init(Window* window, std::string&& windowName);

    void draw(Scene* scene);

    void initMeshes(Scene* scene);
    
    void cleanup();

    /// - - VMA 
    void generateVmaDump();

    bool& getWindowResized() { return this->windowResized; }
};
