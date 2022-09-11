#pragma once
#include <vulkan/vulkan.hpp>
#include "Utilities.h"
#include "VulkanWindow.h"
#include "imgui.h"              // Need to be included in header
#include "backends/imgui_impl_sdl.h"     // Need to be included in header

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/fwd.hpp"

#include "VulkanWindow.h"
#include "Model.h"

#include "vk_mem_alloc.h"
//#include "vk_mem_alloc.h"
#ifndef VENGINE_NO_PROFILING
#include "tracy/Tracy.hpp"
#include "tracy/TracyVulkan.hpp"
#endif 

#include <functional>
using stbi_uc = unsigned char;
using SDL_Events = std::vector<SDL_Event>;
class VulkanRenderer {
#ifndef VENGINE_NO_PROFILING
    std::vector<TracyVkCtx> tracyContext;
#endif
    VulkanWindow window{};

    VmaAllocator vma = nullptr;

    int currentFrame = 0; 

    bool windowResized = false;
    /// Scene Objects
    //std::vector<Mesh> meshes;

    /// Scene Settings
    struct UboViewProjection { /// Model View Projection struct
        glm::mat4 projection;   /// How the Camera Views the world (Ortographic or Perspective)
        glm::mat4 view;         /// Where our Camera is viewing from and the direction it is viewing        
    }uboViewProjection{};

    ///Vulkan Components
    /// - Main
    
    //vk::UniqueInstance               instance{}; /// The Core of Vulkan Program, gives access to the GPU
    vk::Instance               instance{}; /// The Core of Vulkan Program, gives access to the GPU
    vk::DispatchLoaderDynamic dynamicDispatch;
    vk::DebugUtilsMessengerEXT debugMessenger{}; /// used to handle callback from errors based on the validation Layer (??)
    VkDebugReportCallbackEXT debugReport{};
    struct {
        vk::PhysicalDevice physicalDevice;
        vk::Device         logicalDevice;
    } mainDevice{};
    QueueFamilyIndices    QueueFamilies;
    vk::Queue             graphicsQueue{};      /// Also acts as the TransferQueue 
    vk::Queue             presentationQueue{};
    vk::SurfaceKHR        surface{};            ///Images will be displayed through a surface, which GLFW will read from
    vk::SwapchainKHR      swapChain = VK_NULL_HANDLE;

    std::vector<SwapChainImage>  swapChainImages;         /// We store ALL our SwapChain images here... to have access to them
    std::vector<vk::Framebuffer>   swapChainFrameBuffers;
    std::vector<vk::CommandBuffer> commandBuffers;
    /*! The Swapchain image, Framebuffer and Commandbuffers are all 1:1,
     *  i.e. sweapChainImage[1] will only be used by swapChainFrameBuffers[1] which will only be used by commandBuffers[1]
     * */

    std::vector<vk::Image>        colorBufferImage;
    //std::vector<vk::DeviceMemory> colorBufferImageMemory;
    std::vector<VmaAllocation> colorBufferImageMemory;
    std::vector<vk::ImageView>    colorBufferImageView;
    vk::Format                  colorFormat{};

    std::vector<vk::Image>        depthBufferImage;
    //std::vector<vk::DeviceMemory> depthBufferImageMemory;
    std::vector<VmaAllocation> depthBufferImageMemory;
    std::vector<vk::ImageView>    depthBufferImageView;
    vk::Format                    depthFormat{};

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

    std::vector<vk::Buffer>       viewProjection_uniformBuffer;
    //std::vector<vk::DeviceMemory> viewProjection_uniformBufferMemory;
    std::vector<VmaAllocation> viewProjection_uniformBufferMemory;
    std::vector<VmaAllocationInfo> viewProjection_uniformBufferMemory_info;

    std::vector<vk::Buffer>       model_dynamicUniformBuffer;
    //std::vector<vk::DeviceMemory> model_dynamicUniformBufferMemory;
    std::vector<VmaAllocation> model_dynamicUniformBufferMemory;

    // - Assets    

    std::vector<Model>  modelList;

    std::vector<vk::Image>        textureImages;
    //std::vector<vk::DeviceMemory> textureImageMemory;
    std::vector<VmaAllocation> textureImageMemory;
    std::vector<vk::ImageView>    textureImageViews;


    /// Left for Refernce; We do not use Dynamic Uniform Buffer for our Model Matrix, instead we use Push Constants...
    ///vk::DeviceSize    minUniformBufferOffset;
    ///size_t          modelUniformAlignment;
    ///Model*          modelTransferSpace;

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
    vk::Format            swapChainImageFormat{};
    vk::Extent2D          swapChainExtent{};         /// Surface Extent...    

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
    vk::DescriptorPool  descriptorPool_imgui;
    std::vector<vk::CommandPool>     commandPools_imgui;
    std::vector<vk::CommandBuffer>   commandBuffers_imgui;
    std::vector<vk::Framebuffer>     frameBuffers_imgui;

    /// - - VMA 
    void generateVmaDump();

    /// - - Tracy
    void registerVkObjectDbgInfo(std::string name, vk::ObjectType type, uint64_t objectHandle);

#ifndef VENGINE_NO_PROFILING    
    void initTracy();
    void allocateTracyImageMemory();
    void getFrameThumbnailForTracy();
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
    void createInstance();
    void createLogicalDevice();
    void setupDebugMessenger();
    void createSurface();
    void createSwapChain();
    void reCreateSwapChain();    
    void createRenderPass_Base();
    void createRenderPass_Imgui();
    void createDescriptorSetLayout();
    void createPushConstantRange();
    void createGraphicsPipeline_Base();
    void createGraphicsPipeline_Imgui();
    void createGraphicsPipeline_DynamicRendering();
    void createColorBufferImage_Base();    
    void createDepthBufferImage();    
    void createFrameBuffers();
    void createCommandPool();   //TODO: Deprecate! 
    void createCommandBuffers(); //TODO: Deprecate!  //Allocate Command Buffers from Command pool...
    void createSynchronisation();
    void createTextureSampler();

    void createUniformBuffers();
    void createDescriptorPool();
    void createDescriptorSets();
    void allocateDescriptorSets();
    void createInputDescriptorSets();

    // Cleanup 
    void cleanupSwapChain();
    void cleanColorBufferImage_Base();
    void cleanDepthBufferImage();
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
    void recordRenderPassCommands_imgui(uint32_t currentImageIndex);    /// Using renderpass
    void recordRenderPassCommands_Base(uint32_t currentImageIndex);    /// Using renderpass
    void recordDynamicRenderingCommands(uint32_t currentImageIndex);   /// Using DynamicRendering

    /// - Get Functions
    void getPhysicalDevice();

    /// - Allocate Functions
    void allocateDynamicBufferTransferSpace();

    /// - Support Functions
    /// -- Checker Functions
    static bool checkInstanceExtensionSupport(std::vector<const char*>* checkExtensions);
    static bool checkDeviceExtensionSupport(vk::PhysicalDevice device);
    bool checkDeviceSuitable(vk::PhysicalDevice device);


    /// -- Getter Functions
    QueueFamilyIndices getQueueFamilies(vk::PhysicalDevice device);
    SwapChainDetails   getSwapChainDetails(vk::PhysicalDevice device);

    /// -- Choose Functions
    static vk::SurfaceFormat2KHR chooseBestSurfaceFormat(const std::vector<vk::SurfaceFormat2KHR> &formats );
    static vk::PresentModeKHR    chooseBestPresentationMode(const std::vector<vk::PresentModeKHR> &presentationModes);
    vk::Extent2D                 chooseBestImageResolution(const vk::SurfaceCapabilities2KHR & surfaceCapabilities);    
    [[nodiscard]] vk::Format     chooseSupportedFormat(const std::vector<vk::Format> &formats, vk::ImageTiling tiling, vk::FormatFeatureFlagBits featureFlags) const;

    /// -- Create Functions    
    [[nodiscard]]vk::Image         createImage(createImageData &&imageData,  const std::string &imageDescription) ;
    vk::ImageView     createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags) const;
    [[nodiscard]] vk::ShaderModule  createShaderModule(const std::vector<char> &code) const;

    int createTextureImage(const std::string &filename);
    int createTexture(const std::string &filename);
    int createTextureDescriptor(vk::ImageView textureImage);

    /// -- Loader Functions
    static stbi_uc*    loadTextuerFile(const std::string &filename, int* width, int* height, vk::DeviceSize* imageSize );

    void rendererGameLoop();
    void draw();

private: 
    /// Clients Privates 
    SDL_Events eventBuffer;
    std::function<void(SDL_Events&)> gameLoopFunction;
    static void populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT &createInfo);

public:
    VulkanRenderer();
    ~VulkanRenderer()   = default;    
    VulkanRenderer(const VulkanRenderer &ref)              = delete;
    VulkanRenderer(VulkanRenderer &&ref)                   = delete;
    VulkanRenderer& operator=(const VulkanRenderer &ref)   = delete;
    VulkanRenderer& operator=(VulkanRenderer &&ref)        = delete;

    int  init(std::string&& windowName);
    int  createModel(const std::string &modelFile);
    void updateModel(int modelIndex, glm::mat4 newModel);
    void registerGameLoop(std::function<void(SDL_Events&)> gameLoopFunc);
    
    void cleanup();
};
