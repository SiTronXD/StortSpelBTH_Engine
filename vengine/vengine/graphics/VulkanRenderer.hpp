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

#include "glm/gtc/matrix_transform.hpp"
#include "glm/fwd.hpp"

#include "Model.hpp"

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

    Window* window;

    VmaAllocator vma = nullptr;

    int currentFrame = 0; 

    bool windowResized = false;
    // Scene Objects
    //std::vector<Mesh> meshes;

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

    vk::SurfaceKHR surface{};            //Images will be displayed through a surface, which GLFW will read from
    
    Swapchain swapchain;
    
    vk::Sampler textureSampler{}; // Sampler used to sample images in order to present (??)

    // - Assets    
    std::vector<Model>  modelList;

    std::vector<vk::Image>        textureImages;
    std::vector<VmaAllocation> textureImageMemory;
    std::vector<vk::ImageView>    textureImageViews;

    vk::RenderPass renderPassBase{};
    vk::RenderPass renderPassImgui{};
    vk::CommandPool commandPool{};
    CommandBufferArray commandBuffers;

    UniformBufferID viewProjectionUB;

    SamplerID sampler0;

    ShaderInput shaderInput;
    Pipeline pipeline;

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
    // - - Tracy  Callbacks
    bool TracyThumbnail_bool = false;
#endif
public:
#ifndef VENGINE_NO_PROFILING    
    // TODO: this should not be visible to client...
    // void toggleTracyThumbnail(bool toggle){
    //     TracyThumbnail_bool = toggle;
    // };
#endif

private:

    //Vulkan Functions
    // - Create functions
    void setupDebugMessenger();
    void createSurface();
    void recreateSwapchain(Camera* camera);
    void createRenderPassBase();
    void createRenderPassImgui();
    void createCommandPool();   //TODO: Deprecate! 
    void createSynchronisation();
    void createTextureSampler();

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

    int  init(Window* window, std::string&& windowName);
    int  createModel(const std::string &modelFile);
    void updateModel(int modelIndex, glm::mat4 newModel);
    void draw(Scene* scene);

    void initMeshes(Scene* scene);
    
    void cleanup();

    // - - VMA 
    void generateVmaDump();

    bool& getWindowResized() { return this->windowResized; }
};
