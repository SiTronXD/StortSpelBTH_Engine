#include "VulkanRenderer.h"
#include "Utilities.h"
#include "assimp/Importer.hpp"
#include "defs.h"
#include "VulkanValidation.h"
#include "tracyHelper.h"
#include "Configurator.h"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <utility>
#include <fstream>
#include <set>
#include <vector>
#include <limits>               /// Used to get the Max value of a uint32_t
#include <algorithm>            /// Used for std::clamp...
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include "stb_image.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "backends/imgui_impl_vulkan.h"

using namespace vengine_helper::config;
int VulkanRenderer::init(std::string&& windowName) {
    
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    
    this->window.initWindow(windowName, DEF<int>(W_WIDTH), DEF<int>(W_HEIGHT));
    this->window.registerResizeEvent(windowResized);
    try {
        createInstance();       /// Order is important!

        /// - Will create a Instance that checks if we have all needed extensions
        /// -- for Example if GLFW window surface has support
        createSurface();        /// Will be used by the creation of the Logical Device (??)

        ///Will Fetch (and store pointer to) the first Device which also supports all our requirements
        /// - Runs our function checkDeviceSuitable upon the device
        /// --  This function Fetches all the Physical Device Properties and then checks if
        ///     the device has the required QueueFamilies;
        /// ---    A queueFamily of type GraphicsQueue which also has the Presentation 'feature/ability'(??)
        getPhysicalDevice();    /// Relies on that Instance exists

        ///Based on the requirements tested when we fetched the Physical device we can now
        /// - create the Logical Device; i.e.
        /// -- getPhysicalDevice: Checks that required Extensions and needed QueueFamilies are available, then picks the most suitable Physical Device
        /// -- createLogicalDevice: Creates a interface to the Physical Device!
        createLogicalDevice();  /// Relies on that PhysicalDevice existsfor(size_t i = 0; i < swapChainImages.size();i++)
    
             
        registerVkObjectDbgInfo("Surface",vk::ObjectType::eSurfaceKHR, reinterpret_cast<uint64_t>(vk::SurfaceKHR::CType(this->surface)));                
        registerVkObjectDbgInfo("PhysicalDevice",vk::ObjectType::ePhysicalDevice, reinterpret_cast<uint64_t>(vk::PhysicalDevice::CType(this->mainDevice.physicalDevice)));                
        registerVkObjectDbgInfo("LogicalDevice",vk::ObjectType::eDevice, reinterpret_cast<uint64_t>(vk::Device::CType(this->mainDevice.logicalDevice)));

        VmaAllocatorCreateInfo vmaAllocatorCreateInfo{};
        vmaAllocatorCreateInfo.device = this->mainDevice.logicalDevice;
        vmaAllocatorCreateInfo.physicalDevice = this->mainDevice.physicalDevice;
        vmaAllocatorCreateInfo.instance = this->instance;
        vmaAllocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
        if(vmaCreateAllocator(&vmaAllocatorCreateInfo, &this->vma) != VK_SUCCESS){
            throw std::runtime_error("Could not create the VMA (vulkan memory allocator)!");
        }

        setupDebugMessenger();  /// Used when we use Validation Layers to trigger errors/warnings/etc.

        createSwapChain();
        
        this->createColorBufferImage_Base();

        this->createDepthBufferImage();

        createRenderPass_Base();
        createRenderPass_Imgui();
        createDescriptorSetLayout();
        
        this->createPushConstantRange();    /// Create the pushConstantRange that is used by the PipelineLayoutCreateInfo

        createGraphicsPipeline_Base();                     /// Creates pipeline used with RenderPasses...        
        //this->createDynamicRenderingGraphicsPipeline(); /// Creates pipeline used for Dynamic Rendering
        
        createFrameBuffers();
        createCommandPool();
        
        createCommandBuffers(); 

        this->createTextureSampler();
        
        /// Left for Reference; We do not use the Dynamic Uniform Buffer anymore since Push Constants was better suited
        //// for frequently updating data such as the Model Matrix! 
        /// this->allocateDynamicBufferTransferSpace();   /// set value of this->modelUniformAlignment

        this->createUniformBuffers();

        this->createDescriptorPool();

        this->allocateDescriptorSets();
        this->createDescriptorSets();
        
        this->createInputDescriptorSets();

        ///! recordCommands(); <-- This is Moved since we now will recordCommands every single drawCall!

        this->createSynchronisation();        

        this->updateUBO_camera_Projection(); //TODO: Allow for more cameras! 
        this->updateUBO_camera_view(
            glm::vec3(DEF<float>(CAM_EYE_X),DEF<float>(CAM_EYE_Y),DEF<float>(CAM_EYE_Z)),
            glm::vec3(DEF<float>(CAM_TARGET_X),DEF<float>(CAM_TARGET_Y), DEF<float>(CAM_TARGET_Z)));

        ///Change: The MVP is now uboViewProjection, now we store the Model Matrix in the Mesh Class!                             
        // uboViewProjection.model       =                         /// Model Matrix defines where the object is located in the World
        //                     glm::mat4(1.F);       /// Identity Matrix; means the model location will remain 


        /// Inverting the Y-Coordinate on our Projection Matrix so GLM will work with Vulkan!        
                                        /// However Vulkan uses LeftHanded, so it inverts the Y Coordinate ; Positive Y is considered Down dir


        /// Setup Fallback Texture: Let first Texture be default if no other texture is found.
        createTexture("missing_texture.png");

#ifndef VENGINE_NO_PROFILING
        initTracy();        
#endif
        initImgui();
    


        /*
        // This is how the Vertices looked before we use indexes
        std::vector<Vertex> meshVertices {

            // First Triangle!
            {{0.4,-0.4,0.0}, {1.0F,0.0F,0.0F}},     // 0
            {{0.4, 0.4,0.0}, {0.0F,1.0F,0.0F}},     // 1
            {{-0.4,0.4,0.0}, {0.0F,0.0F,1.0F}},     // 2

            // Second Triangle!
            {{-0.4,0.4,0.0} , {0.0F,0.0F,1.0F} },   // 2
            {{-0.4,-0.4,0.0}, {1.0F,1.0F,0.0F} },   // 3
            {{0.4,-0.4,0.0} , {1.0F,0.0F,0.0F} },   // 0
        };
        */


        /// Create a Mesh
        // Vertex Data
        /*
        std::vector<Vertex> meshVertices {          // Only store unique vertexes!
        /// POSITION             COLOR                  UV 
            {{-0.4,  0.4,  0.0}, { 1.0F,  0.0F,  0.0F}, {1.F, 1.F}},     // 0
            {{-0.4, -0.4,  0.0}, { 1.0F,  0.0F,  0.0F}, {1.F, 0.F}},     // 1
            {{ 0.4, -0.4,  0.0}, { 1.0F,  0.0F,  0.0F}, {0.F, 0.F}},     // 2
            {{ 0.4,  0.4,  0.0}, { 1.0F,  0.0F,  0.0F}, {0.F, 1.F}},     // 3
        };
        std::vector<Vertex> meshVertices2 {          // Only store unique vertexes!
            {{-0.25,  0.6,  0.0}, { 0.0F,  1.0F,  0.0F}, {1.F, 1.F}},     // 0
            {{-0.25, -0.6,  0.0}, { 0.0F,  1.0F,  0.0F}, {1.F, 0.F}},     // 1
            {{ 0.25, -0.6,  0.0}, { 0.0F,  1.0F,  0.0F}, {0.F, 0.F}},     // 2
            {{ 0.25,  0.6,  0.0}, { 0.0F,  1.0F,  0.0F}, {0.F, 1.F}},     // 3
        };

        // Index Data
        std::vector<uint32_t> meshIndicies {
            0,  1,  2,                      /// First  Triangle
            2,  3,  0                       /// Second Triangle
        };

        meshes.emplace_back(
            Mesh(this->mainDevice.physicalDevice, 
            this->mainDevice.logicalDevice,
            this->graphicsQueue,
            this->graphicsCommandPool, 
            &meshVertices,
            &meshIndicies,
            this->createTexture("gamepad.png")));

        meshes.emplace_back(
            Mesh(this->mainDevice.physicalDevice, 
            this->mainDevice.logicalDevice,
            this->graphicsQueue,
            this->graphicsCommandPool, 
            &meshVertices2,
            &meshIndicies,
            this->createTexture("gamepad.png")));
        */

        


    }
    catch(std::runtime_error &e)
    {
        std::cout << "ERROR: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void VulkanRenderer::updateModel(int modelIndex, glm::mat4 newModel)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    modelList[modelIndex].setModelMatrix(newModel);
}

void VulkanRenderer::registerGameLoop(std::function<void(SDL_Events&)> gameLoopFunc)
{
    this->gameLoopFunction = gameLoopFunc;
    this->rendererGameLoop();
}

void VulkanRenderer::generateVmaDump()
{
    char* vma_dump;
    vmaBuildStatsString(this->vma,&vma_dump,VK_TRUE);
    std::ofstream file("vma_dump.json");
    file << vma_dump << std::endl;
    file.close();
    
}


void VulkanRenderer::createInstance() {
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    ///Check if in debug mode and if we can use the Vulkan Validation Layers...
    if (isValidationLayersEnabled() && !Validation::checkValidationLayerSupport()) {
        throw std::runtime_error("Tried to use a non-available validation layer...");
    }

#ifndef NDEBUG
    if(!isValidationLayersEnabled()){
        std::cout << "Warning! in Debug mode, but programmatic warnings are disabled!\n";
    }
#endif
    ///VkApplication info is used by VkInstanceCreateInfo, its info about the application we're making
    //This is used by developers, for things like debugging ...
    vk::ApplicationInfo appInfo = {};
    appInfo.setPApplicationName("My Vulkan App");               /// Custom Name of the application
    appInfo.setApplicationVersion(VK_MAKE_VERSION(1,0,0));      /// Description of which version this is of our program
    appInfo.setPEngineName("My Cool Engine");                   /// Name of the used Engine
    appInfo.setEngineVersion(VK_MAKE_VERSION(1,0,0));           /// Description of which version this is of the Engine    
    appInfo.setApiVersion(VK_API_VERSION_1_3);                  /// Version of the Vulkan API that we are using!
    
    ///Creation information for a vk instance (vulkan instance)
    vk::InstanceCreateInfo createInfo({},&appInfo);

    ///Create vector to hold instance extensions.
    auto instanceExtensions = std::vector<const char*>();

    ///Set up extensions Instance to be used
    unsigned int sdlExtensionCount = 0;        /// may require multiple extension  
	SDL_Vulkan_GetInstanceExtensions(this->window.sdl_window, &sdlExtensionCount, nullptr);
    
    ///Store the extensions in sdlExtensions, and the number of extensions in sdlExtensionCount
    std::vector<const char*> sdlExtensions (sdlExtensionCount);    
    
    /// Get SDL Extensions
    SDL_Vulkan_GetInstanceExtensions(this->window.sdl_window, &sdlExtensionCount, sdlExtensions.data());

    /// Add SDL extensions to vector of extensions    
    for (size_t i = 0; i < sdlExtensionCount; i++) {        
        instanceExtensions.push_back(sdlExtensions[i]);    ///One of these extension should be VK_KHR_surface, this is provided by SDL!
    }

    ///Check if any of the instance extensions is not supported...
    if (!checkInstanceExtensionSupport(&instanceExtensions)) {
        throw std::runtime_error("vk::Instance does not support at least one of the required extension!");
    }

    if (isValidationLayersEnabled()) {
        /// When we use Validation Layers we want add a extension in order to get Message Callback...
        instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);        
    }
    
#ifdef DEBUG
    instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);        
#endif 
    //instanceExtensions.push_back(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME); (!!)

    instanceExtensions.insert(instanceExtensions.end(), extraInstanceExtensions.begin(), extraInstanceExtensions.end());

    ///We add the Extensions to the createInfo,
    ///Note: we use static_cast to convert the size_t (returned from size) to uint32_t,
    ///      this is to avoid problems with different implementations...
    createInfo.setEnabledExtensionCount(static_cast<uint32_t>(instanceExtensions.size()));
    createInfo.setPpEnabledExtensionNames(instanceExtensions.data());

    if (isValidationLayersEnabled()) {
        /// The validation layers are defined in my VulkanValidation.h file.
        createInfo.setEnabledLayerCount(static_cast<uint32_t>(validationLayers.size()));
        createInfo.setPpEnabledLayerNames(validationLayers.data()); 

    } else {
        createInfo.setEnabledLayerCount(uint32_t(0));       /// Set to zero, right now we do not want any validation layers        
        createInfo.setPpEnabledLayerNames(nullptr);         /// since we don't use validation layer right now
    }
    
    vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    
    vk::ValidationFeaturesEXT validationFeatures{};
    std::vector<vk::ValidationFeatureEnableEXT> disabledValidationFeatures{};
    std::vector<vk::ValidationFeatureEnableEXT> enabledValidationFeatures{ /// TODO: extract to a cfg file for easy configuration
        vk::ValidationFeatureEnableEXT::eBestPractices,
        vk::ValidationFeatureEnableEXT::eDebugPrintf,
        vk::ValidationFeatureEnableEXT::eSynchronizationValidation,
        vk::ValidationFeatureEnableEXT::eGpuAssisted,
        vk::ValidationFeatureEnableEXT::eGpuAssistedReserveBindingSlot,
    };

    ///Enable Debugging in createInstance function (special Case...)
    if (isValidationLayersEnabled()) {        
        populateDebugMessengerCreateInfo(debugCreateInfo);
        
        std::vector<vk::ExtensionProperties> layerProperties = vk::enumerateInstanceExtensionProperties();
                
        validationFeatures.setDisabledValidationFeatureCount(0);
        validationFeatures.setPDisabledValidationFeatures(nullptr);
        validationFeatures.setEnabledValidationFeatureCount(static_cast<uint32_t>(enabledValidationFeatures.size()));
        validationFeatures.setPEnabledValidationFeatures(enabledValidationFeatures.data());
        
        debugCreateInfo.setPNext(&validationFeatures);
        createInfo.setPNext((vk::DebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo);
    }else{
        createInfo.setPNext(nullptr);
    }

    ///Create the instance!
    //this->instance = vk::createInstanceUnique(createInfo); (!!)        
    this->instance = vk::createInstance(createInfo);
    
}


bool VulkanRenderer::checkInstanceExtensionSupport(std::vector<const char *>* checkExtensions) {
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    /// Fetch the instance extensions properties...
    std::vector<vk::ExtensionProperties> extensions = vk::enumerateInstanceExtensionProperties();

    ///Check if the given extensions is in the list of valid extensions
    for (auto &checkExtension : *checkExtensions) {
        bool hasExtension = false;
        for (auto &extension : extensions) {
            if (strcmp(checkExtension, extension.extensionName ) == 0) {
                hasExtension = true;
                break;
            }
        }
        if (!hasExtension) {
            return false;
        }
    }

    return true;
}

bool VulkanRenderer::checkDeviceExtensionSupport(vk::PhysicalDevice device) {
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    /// Fetch the Physical Device Extension properties...
    std::vector<vk::ExtensionProperties> device_extensionProperties = device.enumerateDeviceExtensionProperties();

    /// Return false if Physical Device does not support any Extensions
    if (device_extensionProperties.empty()) {
        return false;
    }

    /// Make sure all requested extensions exists on the Physical Device
    for (const auto &requested_extensionProperty : deviceExtensions ) {            

        bool required_extension_exists = false;
        for (const auto &device_extensionProperty : device_extensionProperties ) {  
            if (strcmp(requested_extensionProperty, device_extensionProperty.extensionName) == 0) {
                required_extension_exists = true;
                break;
            }
        }
        if (!required_extension_exists) {
            return false;
        }
    }

    return true;
}
 
void VulkanRenderer::cleanup()
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

#ifndef VENGINE_NO_PROFILING
    tracy::GetProfiler().RequestShutdown(); //TODO: is this correct?    
#endif
    
    //Wait until no actions is run on device...
    vkDeviceWaitIdle(mainDevice.logicalDevice); /// Dont destroy semaphores before they are done
    
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    this->mainDevice.logicalDevice.destroyRenderPass(this->renderPass_imgui);
    this->mainDevice.logicalDevice.destroyDescriptorPool(this->descriptorPool_imgui);
    
    for(auto& imgui_cmd_pool : this->commandPools_imgui){
        this->mainDevice.logicalDevice.destroyCommandPool(imgui_cmd_pool);
    }    
    for(auto& imgui_framebuffer : this->frameBuffers_imgui){
        this->mainDevice.logicalDevice.destroyFramebuffer(imgui_framebuffer);
    }

    /// Left as Refence; we dont use a Dynamic Uniform Buffer to update the Model Matrix anymore
    ///free(modelTransferSpace); // Free the memory allocated with aligned_alloc     

#ifndef VENGINE_NO_PROFILING
    CustomFree(this->tracyImage);

    for(size_t i = 0 ; i < this->swapChainImages.size(); i++){
        TracyVkDestroy(this->tracyContext[i]);
    }
#endif

    for(auto & i : modelList)
    {
        i.destroryMeshModel();
    }

    this->mainDevice.logicalDevice.destroyDescriptorPool(this->inputDescriptorPool);
    this->mainDevice.logicalDevice.destroyDescriptorSetLayout(this->inputSetLayout);

    this->mainDevice.logicalDevice.destroyDescriptorPool(this->samplerDescriptorPool);
    this->mainDevice.logicalDevice.destroyDescriptorSetLayout(this->samplerDescriptorSetLayout);

    this->mainDevice.logicalDevice.destroySampler(this->textureSampler);

    for(size_t i = 0; i < this->textureImages.size();i++){
        this->mainDevice.logicalDevice.destroyImageView(this->textureImageViews[i]);
        this->mainDevice.logicalDevice.destroyImage(this->textureImages[i]);
        //this->mainDevice.logicalDevice.freeMemory(this->textureImageMemory[i]);
        vmaFreeMemory(this->vma,this->textureImageMemory[i]);
    }

    for(size_t i = 0; i < this->depthBufferImage.size(); i++ )
    {
        this->mainDevice.logicalDevice.destroyImageView(this->depthBufferImageView[i]);
        this->mainDevice.logicalDevice.destroyImage(this->depthBufferImage[i]);
        //this->mainDevice.logicalDevice.freeMemory(this->depthBufferImageMemory[i]);
        vmaFreeMemory(this->vma,this->depthBufferImageMemory[i]);
    }

    for(size_t i = 0; i < this->colorBufferImage.size(); i++ )
    {
        this->mainDevice.logicalDevice.destroyImageView(this->colorBufferImageView[i]);
        this->mainDevice.logicalDevice.destroyImage(this->colorBufferImage[i]);
        //this->mainDevice.logicalDevice.freeMemory(this->colorBufferImageMemory[i]);
        vmaFreeMemory(this->vma,this->colorBufferImageMemory[i]);
    }

    this->mainDevice.logicalDevice.destroyDescriptorPool(this->descriptorPool);
    this->mainDevice.logicalDevice.destroyDescriptorSetLayout(this->descriptorSetLayout);

    for(size_t i = 0; i < this->swapChainImages.size();i++){
        this->mainDevice.logicalDevice.destroyBuffer(this->viewProjection_uniformBuffer[i]);
        //this->mainDevice.logicalDevice.freeMemory(this->viewProjection_uniformBufferMemory[i]);
        vmaFreeMemory(this->vma, this->viewProjection_uniformBufferMemory[i]);

/*      /// Left as Refence; we dont use a Dynamic Uniform Buffer to update the Model Matrix anymore
        vkDestroyBuffer(this->mainDevice.logicalDevice, this->model_dynamicUniformBuffer[i], nullptr);
        vkFreeMemory(this->mainDevice.logicalDevice, this->model_dynamicUniformBufferMemory[i], nullptr);
*/
    }

    for(int i = 0; i < MAX_FRAME_DRAWS; i++){
        this->mainDevice.logicalDevice.destroySemaphore(this->renderFinished[i]);
        this->mainDevice.logicalDevice.destroySemaphore(this->imageAvailable[i]);
        this->mainDevice.logicalDevice.destroyFence(this->drawFences[i]);        
    }

    this->mainDevice.logicalDevice.destroyCommandPool(graphicsCommandPool);
    for (auto framebuffer: this->swapChainFrameBuffers) {
        this->mainDevice.logicalDevice.destroyFramebuffer(framebuffer);
        
    }

    this->mainDevice.logicalDevice.destroyPipelineCache(this->graphics_pipelineCache);
    this->mainDevice.logicalDevice.destroyPipeline(this->secondGraphicsPipeline);
    this->mainDevice.logicalDevice.destroyPipelineLayout(this->secondPipelineLayout);

    this->mainDevice.logicalDevice.destroyPipeline(this->graphicsPipeline);
    this->mainDevice.logicalDevice.destroyPipelineLayout(this->pipelineLayout);
    this->mainDevice.logicalDevice.destroyRenderPass(this->renderPass_base);
    for (auto image : swapChainImages) {
        mainDevice.logicalDevice.destroyImageView(image.imageView);
    }
    this->mainDevice.logicalDevice.destroySwapchainKHR(this->swapChain);
    this->instance.destroy(this->surface); //NOTE: No warnings/errors if we run this line... Is it useless? Mayber gets destroyed by SDL?

    vmaDestroyAllocator(this->vma);

    this->mainDevice.logicalDevice.destroy();

    if (isValidationLayersEnabled()) {
        this->instance.destroyDebugUtilsMessengerEXT(this->debugMessenger,nullptr, this->dynamicDispatch);
    }

    //this->instance->destroy();   // <- I think this gets taken care of by the destructor... maybe? 
    
}

void VulkanRenderer::draw()
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped;
    const char* const draw_frame = "Draw Frame";
    FrameMarkStart(draw_frame);        
#endif    
    {
        #ifndef VENGINE_NO_PROFILING
        ZoneNamedN(draw_zone1, "Wait for fences", true); //:NOLINT   
        #endif 

        ImGui::Render();
        
        ///TODO: PROFILING; Check if its faster to have wait for fences after acquire image or not...
        /// Wait for The Fence to be signaled from last Draw for this currrent Frame; 
        //// This will freeze the CPU operations here and wait for the Fence to open
        vk::Bool32 waitForAllFences = VK_TRUE;

        auto result = this->mainDevice.logicalDevice.waitForFences(
            uint32_t(1),                        /// number of Fences to wait on
            &this->drawFences[currentFrame],    /// Which Fences to wait on
            waitForAllFences,                   /// should we wait for all Fences or not?              
            std::numeric_limits<uint64_t>::max());
        if(result != vk::Result::eSuccess) {throw std::runtime_error("Failed to wait for all fences!");}        
    }

    unsigned int imageIndex = 0 ;
    {
        #ifndef VENGINE_NO_PROFILING
        ZoneNamedN(draw_zone2, "Get Next Image", true); //:NOLINT   
        #endif 
        // -- Get Next Image -- 
        //1. Get Next available image image to draw to and set a Semaphore to signal when we're finished with the image 

        vk::Result result{};
        /// Retrieve the Index of the image to be displayed.
        std::tie(result, imageIndex) = this->mainDevice.logicalDevice.acquireNextImageKHR( 
            this->swapChain,
            std::numeric_limits<uint64_t>::max(),   /// How long to wait before the Image is retrieved, crash if reached. 
                                                    //// We dont want to use a timeout, so we make it as big as possible.
            this->imageAvailable[currentFrame],     /// The Semaphore to signal, when it's available to be used!
            VK_NULL_HANDLE                          /// The Fence to signal, when it's available to be used...(??)
        );
        if(result == vk::Result::eErrorOutOfDateKHR){
            reCreateSwapChain();    
            return;
        }
        else if(result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {throw std::runtime_error("Failed to AcquireNextImage!");}

        /// Close the Fence behind us if work is being submitted...
        result = this->mainDevice.logicalDevice.resetFences(
            uint32_t(1),
            &this->drawFences[currentFrame]);
        if(result != vk::Result::eSuccess) {throw std::runtime_error("Failed to reset fences!");}
    }
    
    /// ReRecord the current CommandBuffer! In order to update any Push Constants
    recordRenderPassCommands_Base(imageIndex);
    recordRenderPassCommands_imgui(imageIndex);
    //recordDynamicRenderingCommands(imageIndex); ///TODO: User should be able to set if DynamicRendering or Renderpass should be used
    
    /// Update the Uniform Buffers
    this->updateUniformBuffers(imageIndex);

    {
        #ifndef VENGINE_NO_PROFILING
        ZoneNamedN(draw_zone3, "Wait for Semaphore", true); //:NOLINT   
        #endif 

        // -- Submit command buffer to Render -- 
        //2. Submit command buffer to queue for execution, making sure it waits for the image to be signalled as 
        //   available before drawing and signals when it has finished renedering. 
        
        std::array<vk::PipelineStageFlags2, 1> waitStages = {               /// Definies What stages the Semaphore have to wait on.        
            vk::PipelineStageFlagBits2::eColorAttachmentOutput  /// Stage: Start drawing to the Framebuffer...
        };
        
        vk::SemaphoreSubmitInfo wait_semaphoreSubmitInfo;
        wait_semaphoreSubmitInfo.setSemaphore(this->imageAvailable[currentFrame]);
        wait_semaphoreSubmitInfo.setStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput); // (!!)
        //wait_semaphoreSubmitInfo.setStageMask(vk::PipelineStageFlagBits2::eAllCommands); // (!!)(??)
        wait_semaphoreSubmitInfo.setDeviceIndex(uint32_t(1));                            // 0: sets all devices in group 1 to valid... bad or good?

        vk::SemaphoreSubmitInfo signal_semaphoreSubmitInfo;
        signal_semaphoreSubmitInfo.setSemaphore(this->renderFinished[currentFrame]);
        signal_semaphoreSubmitInfo.setStageMask(vk::PipelineStageFlags2());      /// Stages to check semaphores at    

        std::vector<vk::CommandBufferSubmitInfo> commandBufferSubmitInfos{
            vk::CommandBufferSubmitInfo{this->commandBuffers[imageIndex]},
            vk::CommandBufferSubmitInfo{this->commandBuffers_imgui[imageIndex]}
        };        
        
        vk::SubmitInfo2 submitInfo {};      
        submitInfo.setWaitSemaphoreInfoCount(uint32_t(1));
        //!!!submitInfo.setWaitSemaphoreInfos(const vk::ArrayProxyNoTemporaries<const vk::SemaphoreSubmitInfo> &waitSemaphoreInfos_)
        submitInfo.setPWaitSemaphoreInfos(&wait_semaphoreSubmitInfo); /// Pointer to the semaphore to wait on.
        submitInfo.setCommandBufferInfoCount(commandBufferSubmitInfos.size()); 
        submitInfo.setPCommandBufferInfos(commandBufferSubmitInfos.data()); /// Pointer to the CommandBuffer to execute
        submitInfo.setSignalSemaphoreInfoCount(uint32_t(1));
        submitInfo.setPSignalSemaphoreInfos(&signal_semaphoreSubmitInfo);/// Semaphore that will be signaled when 
                                                                        //// CommandBuffer is finished


        //Submit The CommandBuffers to the Queue to begin drawing to the framebuffers
        this->graphicsQueue.submit2(submitInfo,drawFences[currentFrame]); //// drawing, signal this Fence to open!
    }
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();        
            ImGui::RenderPlatformWindowsDefault();
        }
    {
        #ifndef VENGINE_NO_PROFILING
        ZoneNamedN(draw_zone4, "Present Image", true); //:NOLINT   
        #endif 
        // -- Present Rendered Image to Screen -- 
        //3. Present image to screen when it has signalled finished rendering.
        vk::PresentInfoKHR presentInfo{};
        presentInfo.setWaitSemaphoreCount(uint32_t (1));
        presentInfo.setPWaitSemaphores(&this->renderFinished[currentFrame]);  /// Semaphore to Wait on before Presenting
        presentInfo.setSwapchainCount(uint32_t (1));    
        presentInfo.setPSwapchains(&this->swapChain);                         /// Swapchain to present the image to
        presentInfo.setPImageIndices(&imageIndex);                            /// Index of images in swapchains to present                

        /// Submit the image to the presentation Queue
        vk::Result resultvk = this->presentationQueue.presentKHR(&presentInfo);
        if (resultvk == vk::Result::eErrorOutOfDateKHR || resultvk == vk::Result::eSuboptimalKHR || this->windowResized )
        {
            this->windowResized = false;       
            reCreateSwapChain();
        }
        else if(resultvk != vk::Result::eSuccess) {throw std::runtime_error("Failed to present Image!");}
    }

#ifndef VENGINE_NO_PROFILING 
    if(this->TracyThumbnail_bool){
        getFrameThumbnailForTracy();
    }
#endif
    /// Update current Frame for next draw!
    this->currentFrame = (this->currentFrame + 1) % MAX_FRAME_DRAWS;    
    
#ifndef VENGINE_NO_PROFILING    
    FrameMarkEnd(draw_frame);
#endif        
}

void VulkanRenderer::getPhysicalDevice() {
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    ///We need to pick which of the systems physical device to be used; Integrated GPU, One of Multiple GPU, External GPU... etc
    auto physicalDevices = this->instance.enumeratePhysicalDevices();

    ///Check if any devices where available, if none then we dont have support for vulkan...
    if (physicalDevices.empty()) {
        throw std::runtime_error("Can't find GPUs that support Vulkan instances...");
    }

    ///Loop through all possible devices and pick the first suitable device!    
    //for (const auto &device : devices) {
    for (const auto &device : physicalDevices) {
        if (checkDeviceSuitable(device)) {
            mainDevice.physicalDevice = device;
            break;
        }
    }
    this->QueueFamilies = this->getQueueFamilies(mainDevice.physicalDevice);
    /// Get properties of our choosen physical device 
    vk::PhysicalDeviceProperties2 deviceProperties = this->mainDevice.physicalDevice.getProperties2(); //TODO: remove or use...
    
/*  /// Left for Reference; we dont use Dynamic Uniform Buffers for our Model Matrix anymore.
    this->minUniformBufferOffset = deviceProperties.limits.minUniformBufferOffsetAlignment;    

*/
}

void VulkanRenderer::allocateDynamicBufferTransferSpace() //NOLINT:TODO: this function should either be removed or do something...
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    /// Understanding The usage of Bitwise Operators here...
/*
    Binary       Decimal
    00100000  =  32  bits    //Bomary 1  to the right side => Smaller values
    01000000  =  64  bits    //Binary 1  to the left side  => Bigger  values
    10000000  =  128 bits
    100000000 =  256 bits

    ---------------
    Binary Representation of Any Power of Two Value
    Any value that is a power of Two is represented with 
    only one 1 and the rest 0 in binary form. 

    Example: 
              Decimal   Binary
        2^0 = 1         00000001    
        2^1 = 2         00000010    
        2^2 = 4         00000100    
        2^3 = 8         00001000    
        2^4 = 16        00010000    

    ---------------
    One Minus Any Power of Two
    Since we know that a Power of Two value consists of only One 1, 
    we can take any Power Of Two Values and subtract one and we 
    will get something useful! 
    Basicly all the 0's before the 1 (going Right To Left)
    will be flipped to 1's, and the current 1 will be flipped to 0.

    Example 
              Decimal   Binary
        2^0 : 1 -1 = 0  00000000    
        2^1 : 2 -1 = 1  00000001    
        2^2 : 4 -1 = 3  00000011    
        2^3 : 8 -1 = 7  00000111    
        2^4 : 16-1 = 15 00001111        
    
    Decimal value representation in Binary
    
    Decimal     Binary
    128         10000000        <- 128 is a power of 2, thus 
    127         01111111
    126         01111110
    125         01111101


    ---------------
    The AND (&) operator
    If both's binary digits on the same location are the same, then we keep it, else replace with 0
    
    Example 1 : 
        (A) Min Uniform Alignment   :  11100000     <- 224 in decimal
        (B) Size Of Model           :  01000000     <- 64  in decimal
        A         &  B          = 01000000  <- Both A and B share the 1 on the 2:nd position, thus we keep
        11100000  &  01000000   = 01000000  <- 64

    Example 2 : 
        (A) Min Uniform Alignment   :  10000000     <- 128 in decimal
        (B) Size Of Model           :  01000000     <- 64  in decimal
        A         &  B          = 00000000  <- Neither of A and B share the 1 on any position, thus we have 0
        10000000  &  01000000   = 00000000  <- 0 
    
    Example 3 : 
        (A) Min Uniform Alignment   :  01111111     <- 127 in decimal
        (B) Size Of Model           :  01000000     <- 64  in decimal
        A         &  B          = 00000000  <- Both A and B share the 1 on the 2:nd position, thus we keep
        01111111  &  01000000   = 00000000  <- 64 in decimal

    ---------------
    The NOT (~) Operator
    All digits will be flipped, 1's will become 0's and 0's will become 1's...
    NOTE!:  What happens to the 0's in front of the last digit? (Reading right to left) 
            They will also become 1's, so the following examples are not exactly true...

    Example 1 : 
        (A)    :  11100000     <- 224 in decimal
        (B)    :  01000000     <- 64  in decimal

        ~(A)   =  00011111     <- 31  in decimal
        ~(b)   =  10111111     <- 191 in decimal

*/    

/* Left for Reference; We do not update our Model matrix with a Dynamic Uniform Buffer anymore
    /// - CALCULATE ALIGNMENT OF MODEL DATA -
    /// This might be overkill and is only needed if the Size of the UboModel (in this case) is bigger... (ours is small)
    //// But this will assure us that it will work on any systems even if they have poor memory sizes för minUniformBufferOffset...
    this->modelUniformAlignment = 
        (sizeof(Model)                    /// Size we want to Align
        + (this->minUniformBufferOffset -1)) /// Ensures the Alignment if trailing 1's occours... 
        & ~(minUniformBufferOffset - 1);     /// Will give the Alignment needed based on size of UboModel
*/
/*
    ---------------
    Example of how the code above works
    as of writing, the size of the UboModel struct is 64, 
    the size of teh minUniformBufferOffset will however change depending on the hardware.

    Example 1 : lets assume the minUniformBufferOffset is 256

        (sizeof(UboModel)                      : 64            = 64       =  01000000
        + (this->minUniformBufferOffset -1))   : 64 + (256-1)  = 319      = 100111111     
        & ~(minUniformBufferOffset - 1);       : 319 & ~(255)  = 0        = 100111111 & ~(11111111) 
                                                                          = 100111111 &   00000000 = 00000000

    Example 2 : lets assume the minUniformBufferOffset is 64

        (sizeof(UboModel)                      : 64           = 64       = 01000000
        + (this->minUniformBufferOffset -1))   : 64 + (64-1)  = 127      = 01111111
        & ~(minUniformBufferOffset - 1);       : 127 & ~(64-1) = 64      = 01111111 & ~(00111111) 
                                                                         = 01111111 &   11000000 = 01000000

    Example 3 : lets assume the minUniformBufferOffset is 32

        (sizeof(UboModel)                      : 64            = 64       = 01000000
        + (this->minUniformBufferOffset -1))   : 64 + (32-1)   = 95       = 01011111
        & ~(minUniformBufferOffset - 1);       : 95 & ~(32-1)  = 64       = 01011111 & ~(00011111) 
                                                                          = 01011111 &   11100000 = 01000000
*/
    
    /* Left for Reference; We do not update our Model matrix with a Dynamic Uniform Buffer anymore
    this->modelTransferSpace = (Model*)aligned_alloc(
                                            modelUniformAlignment,  /// Size of the Alignment
                                            modelUniformAlignment * MAX_OBJECTS); /// How much Memory To Allocate
    */
}

bool VulkanRenderer::checkDeviceSuitable(vk::PhysicalDevice device) {
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    ///Helps us decide if a device is suitable (for our needs...)

    ///Information about the device itself (Id, name, type, vendor, etc...)
    vk::PhysicalDeviceProperties deviceProperties = device.getProperties(); //TODO: Unused, use or remove

    ///deviceProperties will now contain a lot of information about the device;
    /// - apiVersion, driverVersion, vendorID, deviceType, deviceName, limits, sparseProperties
    ///deviceLimits is a struct (VkPhysicalDeviceLimits) that contains a lot of information about the device specs...

    ///Check if the Device has the required Features... 
    bool device_has_suppoerted_features = false;
    vk::PhysicalDeviceFeatures deviceFeatures = device.getFeatures();
    if(deviceFeatures.samplerAnisotropy == VK_TRUE) //TODO: Extract into a vector similar to what we've done with extensions
    {
        device_has_suppoerted_features = true;
    }
    ///deviceFeatures will not contain a big struct with a lot of boolean members that describes if a feature exist;
    /// - robustBufferAccess, fullDrawIndexUint32, imageCubeArray, independentBlend, geometryShader, tesselationShader, and more

    ///Queue Families defines groups of queues which are used to execute commands of certain types.
    /// - Graphics Queue family, Transfer Queue family, Presentation Queue 6Family, etc...
    ///We need to check that the device support the Queue Families which we are going to be using!
    QueueFamilyIndices indices = getQueueFamilies(device);

    bool extensions_supported = checkDeviceExtensionSupport(device);

    SwapChainDetails swapChainDetails = getSwapChainDetails(device);

    /// Once we have the QueueFamily Indices we can return whatever our "isValid" function concludes...
    return indices.isValid() && extensions_supported && swapChainDetails.isValid() && device_has_suppoerted_features;
}

QueueFamilyIndices VulkanRenderer::getQueueFamilies(vk::PhysicalDevice device) {
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    QueueFamilyIndices indices;

    /// Get Queue Families
    std::vector<vk::QueueFamilyProperties2> queueFamilies = device.getQueueFamilyProperties2();

    /*!The VkQueueFamilyProperties Consist of Four members;
     *  - queueFlags : What type of Queue Family this family belongs to, we use bitfield operation to figure this out.
     *  - queueCount : How many Queues that are part of this type of Queue Family
     *  - timestampValidBits and minImageTransferGranularity ...
     * */

    /// Next we will go through the Queue families, but first we need to declare a index that we have to use later on...
    int32_t queueFamilyIndex = 0;

    ///Go through each of the Queue Families available and check if it has atlas one of the required type of queue.
    for (const auto &queueFamily : queueFamilies) {

        ///Check if the FamilyQueue has any queues, and if it has then check if its part of the Queue Graphics family.
        if (queueFamily.queueFamilyProperties.queueCount > 0  && (queueFamily.queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics)) {
            /// Using Bitwise operations we can see if the QueueFamily is part of the Graphics Queue Family,
            /// We can add more bitfield flags to check if its also part of one of the other Families;
            /// - VK_QUEUE_COMPUTE_BIT, VK_QUEUE_TRANSFER_BIT, VK_QUEUE_SPARSE_BINDING_BIT, VK_QUEUE_PROTECTED_BIT and VK_QUEUE_FLAG_BITS_MAX_ENUM

            indices.graphicsFamily = queueFamilyIndex; /// Set the index for the Graphics Family for valid Queue Family...

        }

        ///Check if this current QueueFamily has presentation support
        vk::Bool32 surfaceHasSupport = VK_FALSE;
        surfaceHasSupport = device.getSurfaceSupportKHR(queueFamilyIndex, this->surface);

        if (queueFamily.queueFamilyProperties.queueCount > 0 &&  (surfaceHasSupport != 0U)) {
            indices.presentationFamily = queueFamilyIndex;
        }

        ///Check if the queue Family Indices are in a valid state.
        /// - Is of type GraphicsQueue
        /// - Has the PresentationQueue ability (??)
        if(indices.isValid()){
            break; /// If valid we are done in this loop
        }

        queueFamilyIndex++;
    }

    return indices;
}

void VulkanRenderer::createLogicalDevice() {
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif


    ///Get the queue Family indices for the Chosen physical Device ...
    QueueFamilyIndices indices = getQueueFamilies(this->mainDevice.physicalDevice);

    ///Using a vector to store all the queueCreateInfo structs for each QueueFamily...
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

    ///Using a set which will store all our indices;
    /// - When using a set we can represent 1 index for each QueueFamily without risk of adding the same queueFamily index multiple times!
    std::set<int32_t> queueFamilyIndices{
        indices.graphicsFamily,
        indices.presentationFamily    /// This could be the same Queue Family as the GraphicsQueue Family! thus, we use a set!
    };

    float priority = 1.F;
    /// Queues the Logical device needs to create, and info to do so; Priority 1 is highest, 0 is lowest...
    /// - Note; if the graphicsFamily and presentationFamily is the same, this loop will only do 1 iteration, else 2...
    for (std::size_t i = 0; i < queueFamilyIndices.size();i++ ) {

        vk::DeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.setQueueFamilyIndex(indices.graphicsFamily);         /// The index of the family to create Graphics queue from
        queueCreateInfo.setQueueCount(uint32_t (1));

        /// We can create Multiple Queues, and thus this is how we decide what Queue to prioritize...        
        queueCreateInfo.setPQueuePriorities(&priority); /// Since we only have One priority, we will use a pointer to the priority value...

        /// Add each of the *Unique* queue Family Indices createQueueInfo instances to the queueCreateInfos vector!
        queueCreateInfos.push_back(queueCreateInfo);
    }


    /*!Vulkan Uses Prioritization to know what Queue to give priority to when Multiple Queues runs at the same time...
     * 1 is the highest, 0 is the lowest...
     * */
    
    
    /// Information to create a logical device (Logical devices are most commonly referred to as 'device', rather than 'logical device'...)
    vk::DeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.setQueueCreateInfoCount(static_cast<uint32_t>(queueCreateInfos.size())); /// Number of queueCreateInfos!
    deviceCreateInfo.setPQueueCreateInfos(queueCreateInfos.data());     /// List of queue create infos so that the device can require Queues...
    
    /// The extensions we want to use are stored in our Global deviceCreateInfo array!
    deviceCreateInfo.setEnabledExtensionCount(static_cast<uint32_t>(deviceExtensions.size()));           /// We dont have any logical Device Extensions (not the same as instance extensions...)
    deviceCreateInfo.setPpEnabledExtensionNames(deviceExtensions.data());/// since we dont have any... use nullptr! (List of Enabled Logical device extensions

    ///Physical Device features the logical Device will be using...
    vk::PhysicalDeviceFeatures2 deviceFeatures {};
    deviceFeatures.features.setSamplerAnisotropy(VK_TRUE);             /// Enables the Anisotropy Feature, now a Sampler made for this Device can use Anisotropy!                 
    
    /// Get extension Features    
    vk::PhysicalDeviceSynchronization2Features physicalDeviceSyncFeatures{};
    vk::PhysicalDeviceDynamicRenderingFeatures physicalDeviceDynamicRenderingFeatures{VK_TRUE};
    physicalDeviceSyncFeatures.setPNext(&physicalDeviceDynamicRenderingFeatures);
    deviceFeatures.setPNext(&physicalDeviceSyncFeatures);    
    //this->mainDevice.physicalDevice.getFeatures2(&deviceFeatures); ///NOTE: <- This line enables all possible device features, which can cause performance degradation...
    physicalDeviceSyncFeatures.setSynchronization2(VK_TRUE);

    deviceCreateInfo.setPNext(&deviceFeatures);
                                             
    
    /*!This instance of createInfo does not need the pNext nor flags.
     * But we do need to define :
     *  - pQueueCreateInfo* : Used to describe what Queues to Create for this Logical Device...
     *  - enabledLayerCount and ppEnabledLayerNames : this is only NEEDED IF we are using Vulkan 1.0... (we use 1.1...)
     *  - enabledExtensionCount and ppEnabledExtensionNames : we will need these later...
     *  - pEnabledFeatures : We will need these later...
     * As of right now, we don need anything but the queueCreateInfo pointer...
     * */
    
    /// Create the logical device for the given Physical Device
    this->mainDevice.logicalDevice = this->mainDevice.physicalDevice.createDevice(deviceCreateInfo);    
    registerVkObjectDbgInfo("Logical Device", vk::ObjectType::eDevice, reinterpret_cast<uint64_t>(vk::Device::CType(this->mainDevice.logicalDevice)));

    /// Setup Dynamic Dispatch, in order to use device extensions        
    this->dynamicDispatch = vk::DispatchLoaderDynamic( instance, vkGetInstanceProcAddr, this->mainDevice.logicalDevice );

    /*! After we specified what kind of Queues exist we need to add them to the Logical Device,
     * We do not create these handles as they are something that already Exists on the Physical devices,
     * Thus we 'GetDeviceQueue' rather than create them...
      */

    /// Queues are Created at the same time as the device...
    /// So we want handle to queues:
    vk::DeviceQueueInfo2 graphicsQueueInfo;
    graphicsQueueInfo.setQueueFamilyIndex(static_cast<uint32_t>(indices.graphicsFamily));
    graphicsQueueInfo.setQueueIndex(uint32_t(0));    
    this->graphicsQueue = this->mainDevice.logicalDevice.getQueue2(graphicsQueueInfo);

    /// Add another handle to let the Logical Device handle PresentationQueue... (??)
    vk::DeviceQueueInfo2 presentationQueueInfo;
    presentationQueueInfo.setQueueFamilyIndex(static_cast<uint32_t>(indices.presentationFamily));
    presentationQueueInfo.setQueueIndex(uint32_t(0));        /// Will be positioned at the Queue index 0 for This particular family... (??)
    this->presentationQueue = this->mainDevice.logicalDevice.getQueue2(presentationQueueInfo);
    
    registerVkObjectDbgInfo("Graphics Queue", vk::ObjectType::eQueue, reinterpret_cast<uint64_t>(vk::Queue::CType(this->graphicsQueue)));
    registerVkObjectDbgInfo("Presentation Queue", vk::ObjectType::eQueue, reinterpret_cast<uint64_t>(vk::Queue::CType(this->presentationQueue))); //TODO: might be problematic.. since it can be same as Graphics Queue
    /*! With both a graphicsQueue and a PresentationQueue we have everything needed to use present to our Particular surface!
     * */

    /*!From Given Logical Device, Of given Queue Family, of Given Queue index ( 0 ce only one queue), place reference in given VkQueue (??)
     * */
}

void VulkanRenderer::setupDebugMessenger() {
    if(!isValidationLayersEnabled()){return;}

    /// In this function we define what sort of message we want to receive...
    vk::DebugUtilsMessengerCreateInfoEXT createInfo{};
    populateDebugMessengerCreateInfo(createInfo);
    
    auto result = this->instance.createDebugUtilsMessengerEXT(&createInfo,nullptr,&this->debugMessenger, dynamicDispatch);
    if (result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create Debug Messenger!");
    }
}

void VulkanRenderer::populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT &createInfo) {
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    createInfo.messageSeverity
        = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
                | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
                | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;

    createInfo.messageType
         = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
                 | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
                 | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

    createInfo.setPfnUserCallback(debugCallback)  ;
    createInfo.pUserData = nullptr;
}

void VulkanRenderer::createSurface() {
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    ///Using SDL to create WindowSurface, to make it cross platform
    ///Creates a surface create info struct configured for how SDL handles windows.    
    VkSurfaceKHR sdlSurface{};

    SDL_bool result = SDL_Vulkan_CreateSurface(
                            (this->window.sdl_window),
                            this->instance,                                                        
                            &sdlSurface
                            );
    
    this->surface = sdlSurface;
    
    if (result != SDL_TRUE ) {
        throw std::runtime_error("Failed to create (GLFW) surface.");
    }
    
}

void VulkanRenderer::createSwapChain() {
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    /// Store Old Swapchain, if it exists
    vk::SwapchainKHR oldSwapchain = this->swapChain;

    /// Get swap chain details so we can pick the best settings
    SwapChainDetails swapChainDetails = getSwapChainDetails(mainDevice.physicalDevice);

    ///Find 'optimal' surface values for our swapChain
    /// - 1. Choose best surface Format
    vk::SurfaceFormat2KHR  surfaceFormat = this->chooseBestSurfaceFormat(swapChainDetails.Format);

    /// - 2. Choose best presentation Mode
    vk::PresentModeKHR presentationMode = this->chooseBestPresentationMode(swapChainDetails.presentationMode);

    /// - 3. Chose Swap Chain image Resolution
    vk::Extent2D imageExtent = this->chooseBestImageResolution(swapChainDetails.surfaceCapabilities);

    /// --- PREPARE DATA FOR SwapChainCreateInfo ... ---
    /// Minimum number of images our swapChain should use.
    /// - By setting the minImageCount to 1 more image than the amount defined in surfaceCapabilities we enable Triple Buffering!
    /// - NOTE: we store the 'minImageCount+1' in a variable, we need to check that 'minImageCount+1' is not more than 'maxImageCount'!
    uint32_t imageCount = std::clamp(swapChainDetails.surfaceCapabilities.surfaceCapabilities.minImageCount + 1,
                                     swapChainDetails.surfaceCapabilities.surfaceCapabilities.minImageCount,
                                     swapChainDetails.surfaceCapabilities.surfaceCapabilities.maxImageCount );

    if ( imageCount == 0 ) {
        /// if swapChainDetails.surfaceCapabilities.maxImageCount was 0 then imageCount will now be 0 too.
        /// This CAN happen IF there is no limit on how many images we can store in the SwapChain.
        /// - i.e. maxImageCount == 0, then there is no maxImageCount!
        ///imageCount    = swapChainDetails.surfaceCapabilities.minImageCount + 1; //!! Nope
        imageCount    = swapChainDetails.surfaceCapabilities.surfaceCapabilities.maxImageCount; /// (??)
        /*! We use the max image count if we can, the clamping we did *Seems* to ensure that we don't get a imageCount is not 0...
         * I'm not sure if I'm missing something... this seems redundant.
         * Could I just :
         *  imageCount = (maxImageCount != 0) ? maxImageCount : minImageCount +1 ;
         *
         * Also... Do I always want to use the MaxImageCount just because I can? Or is minImageCount + X a better choice...
         * */
    }

    ///Create the SwapChain Create Info!
    vk::SwapchainCreateInfoKHR  swapChainCreateInfo = {};
    swapChainCreateInfo.setSurface(this->surface);
    swapChainCreateInfo.setImageFormat(surfaceFormat.surfaceFormat.format);
    swapChainCreateInfo.setImageColorSpace(surfaceFormat.surfaceFormat.colorSpace);
    swapChainCreateInfo.setPresentMode(presentationMode);
    swapChainCreateInfo.setImageExtent(imageExtent);
    swapChainCreateInfo.setMinImageCount(uint32_t (imageCount));    
    swapChainCreateInfo.setImageArrayLayers(uint32_t (1));                                    /// Numbers of layers for each image in chain
    swapChainCreateInfo.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment              /// What 'Attachment' are drawn to the image
                                                | vk::ImageUsageFlagBits::eTransferSrc);    /// Tells us we want to be able to use the image, if we want to take a screenshot etc...    

    /*! The imageUsage defines how the image is intended to be used, there's a couple different options.
     * - vk::ImageUsageFlagBits::eColorAttachment        : Used for Images with colors (??)
     * NOTE: Generally always vk::ImageUsageFlagBits::eColorAttachment...
     *       This is because the SwapChain is used to present images to the screen,
     *       if we wanted to draw a depthBuffer onto the screen, then we could specify another imageUsage flag... (??)
     * */
    swapChainCreateInfo.preTransform       =  swapChainDetails.surfaceCapabilities.surfaceCapabilities.currentTransform;
     /*! Defines the Transform to be used onto the images within the SwapChain!
      * - This is generally the one specific by the surface
      * */
      
    swapChainCreateInfo.compositeAlpha    = vk::CompositeAlphaFlagBitsKHR::eOpaque;    /// Draw as is, no opaque...
      /*! Defines how render should be preformed when we have a overlapping window above the Vulkan Window (??).
       * - 'How to handle Blending images with external graphics (e.g. other windows)'
       * NOTE: I think this is only for opaque windows... (??)
       * */
    swapChainCreateInfo.clipped          = VK_TRUE;                              /// dont draw not visible parts of window
     /*! Defines how render should be preformed when we have a overlapping window,
      * - if a Window overlapps, should we continue to draw the image or stop the rendeirng?
      * - 'Wether to clip parts of image not in view (e.g. behind another window, off screen, etc.)'
      * ==> VK_TRUE : better overall performance ... (??) (I think we're talking about the overall system performance...)
      * NOTE: I think this is only for non opaque windows... (??)
      * */

      //! -- THE TWO DIFFERENT QUEUES - Graphics queue and Presentation Queue --
      /*! The Graphics Queue      : will draw the images contained in our swapChain
       *  The Presentation Queue  : will present the images to the surface (i.e. screen/window)
       *
       *  There are 2 Different modes the swapChain can interact with these Queues;
       *  - Exclusive interaction     : A unique Image from the SwapChain can only be interacted with one queue at the same time
       *  - Concurrent interaction    : A unique Image from the SwapChain can only be interacted with multiple queues at the same time
       *
       *  NOTE: Concurrent interaction is typically slower (for 2 reasons)!
       *  - The Concurrent mode have more overhead
       *  - IF the GraphicsQueue AND PresentationQueue is the SAME queue, then there is no need for multiple queues to interact with the image...
       * */

     /// We pick mode based on if the GraphicsQueue and PresentationQueue is the same queue...
     //TODO: the QueueFamilyIndices should be stored somewhere rather than fetched again...
    QueueFamilyIndices indices = getQueueFamilies(this->mainDevice.physicalDevice);
    std::array<uint32_t ,2> queueFamilies {static_cast<uint32_t>(indices.graphicsFamily),       /// Array of QueueIndices...
                                           static_cast<uint32_t>(indices.presentationFamily)};

    /// If Graphics and Presentation families are different, then SwapChain must let images be shared between families!
    if (indices.graphicsFamily != indices.presentationFamily) {
        
        swapChainCreateInfo.setImageSharingMode(vk::SharingMode::eConcurrent);   /// Use Concurrent mode if more than 1 family is using the swapchain
        swapChainCreateInfo.setQueueFamilyIndexCount(uint32_t (2));                            /// How many different queue families will use the swapchain
        swapChainCreateInfo.setPQueueFamilyIndices(queueFamilies.data());         /// Array containing the queues that will share images
    } else {        
        swapChainCreateInfo.setImageSharingMode(vk::SharingMode::eExclusive);    /// Use Exclusive mode if only one Queue Family uses the SwapChain...
        swapChainCreateInfo.setQueueFamilyIndexCount(uint32_t (0));        /// Note; this is the default value
        swapChainCreateInfo.setPQueueFamilyIndices(nullptr);  /// Note; this is the default value
    }
    

    /*! If we want to create a new SwapChain, this would be needed when for example resizing the window.
     *  with the oldSwapchain we can pass the old swapChains responsibility to the new SwapChain...
     * */
    /// IF old swapChain been destroyed and this one replaces it, then link old one to quickly hand over responsibilities...
    swapChainCreateInfo.setOldSwapchain(oldSwapchain); // VK_NULL_HANDLE on initialization, previous all other times...

    ///Create The SwapChain!    
    this->swapChain = this->mainDevice.logicalDevice.createSwapchainKHR(swapChainCreateInfo);
    registerVkObjectDbgInfo("Swapchain", vk::ObjectType::eSwapchainKHR, reinterpret_cast<uint64_t>(vk::SwapchainKHR::CType(this->swapChain)));

    /*! REMEMBER:
     * All of Vulkans functions labeled with "Create" creates something that will need to be destroyed! ...
     * */

    /// Store both the VkExtent2D and VKFormat, so they can easily be used later...
    swapChainImageFormat    = surfaceFormat.surfaceFormat.format; /// We need this to create a ImageView...
    swapChainExtent         = imageExtent;          /// We need this to create a ImageView...
    /*! ImageView is the interface which we manage image through. (interface to the image... sort of (??))
     * - With an ImageView we can View the Image...
     * */

    /// Get all Images from the SwapChain and store them in our swapChainImages Vector...
    std::vector<vk::Image> images = this->mainDevice.logicalDevice.getSwapchainImagesKHR(this->swapChain);

    uint32_t index = 0;
    for(vk::Image image : images ) {
        /// Copy the Image Handle ...
        SwapChainImage swapChainImage = {};
        swapChainImage.image     = image;

        /// Create the Image View
        swapChainImage.imageView = createImageView(image,swapChainImageFormat, vk::ImageAspectFlagBits::eColor);
        registerVkObjectDbgInfo("Swapchain_ImageView["+std::to_string(index)+"]", vk::ObjectType::eImageView, reinterpret_cast<uint64_t>(vk::ImageView::CType(swapChainImage.imageView)));
        registerVkObjectDbgInfo("Swapchain_Image["+std::to_string(index)+"]", vk::ObjectType::eImage, reinterpret_cast<uint64_t>(vk::Image::CType(swapChainImage.image)));

        index++;
        this->swapChainImages.push_back(swapChainImage);
    }
    
    if(oldSwapchain){
        this->mainDevice.logicalDevice.destroySwapchainKHR(oldSwapchain);
    }
}

void VulkanRenderer::reCreateSwapChain()
{
    vkDeviceWaitIdle(this->mainDevice.logicalDevice);
    
    this->mainDevice.logicalDevice.freeDescriptorSets(this->inputDescriptorPool,this->inputDescriptorSets);
    cleanupFramebuffer_imgui();    
    cleanColorBufferImage_Base();
    cleanDepthBufferImage();
    cleanupSwapChain();
    cleanupRenderBass_Imgui();  
    cleanupRenderBass_Base();  

    createSwapChain();
    createColorBufferImage_Base();
    createDepthBufferImage();    
    createRenderPass_Base();
    createRenderPass_Imgui();

    createFrameBuffers();
    createFramebuffer_imgui();

    this->createDescriptorSets();
    this->createInputDescriptorSets();

    this->updateUBO_camera_Projection();
}

void VulkanRenderer::cleanupSwapChain()
{
    for (auto image : swapChainImages) {
        mainDevice.logicalDevice.destroyImageView(image.imageView);
    }
    swapChainImages.resize(0);

    for (auto framebuffer: this->swapChainFrameBuffers) {
        this->mainDevice.logicalDevice.destroyFramebuffer(framebuffer);        
    }
    this->swapChainFrameBuffers.resize(0);
}

void VulkanRenderer::cleanColorBufferImage_Base()
{
    
    for(size_t i = 0; i < this->colorBufferImage.size(); i++ )
    {
        this->mainDevice.logicalDevice.destroyImageView(this->colorBufferImageView[i]);
        this->mainDevice.logicalDevice.destroyImage(this->colorBufferImage[i]);
        //this->mainDevice.logicalDevice.freeMemory(this->colorBufferImageMemory[i]);
        vmaFreeMemory(this->vma,this->colorBufferImageMemory[i]);
    }
}

void VulkanRenderer::cleanDepthBufferImage()
{
    for(size_t i = 0; i < this->depthBufferImage.size(); i++ )
    {
        this->mainDevice.logicalDevice.destroyImageView(this->depthBufferImageView[i]);
        this->mainDevice.logicalDevice.destroyImage(this->depthBufferImage[i]);
        //this->mainDevice.logicalDevice.freeMemory(this->depthBufferImageMemory[i]);
        vmaFreeMemory(this->vma,this->depthBufferImageMemory[i]);
    }

}

void VulkanRenderer::cleanupRenderBass_Imgui()
{
    this->mainDevice.logicalDevice.destroyRenderPass(this->renderPass_imgui);
}

void VulkanRenderer::cleanupRenderBass_Base()
{
    this->mainDevice.logicalDevice.destroyRenderPass(this->renderPass_base);
}

SwapChainDetails VulkanRenderer::getSwapChainDetails(vk::PhysicalDevice device) {
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    SwapChainDetails swapChainDetails = {};

    /// -- CAPABILITIES --
    /// Get the surface capabilities for the given surface on the given physical device
    vk::PhysicalDeviceSurfaceInfo2KHR physicalDeviceSurfaceInfo{};
    physicalDeviceSurfaceInfo.setSurface(this->surface);

    swapChainDetails.surfaceCapabilities = device.getSurfaceCapabilities2KHR(physicalDeviceSurfaceInfo);

    /// -- FORMATS --
    /// Get the formats which the surface supports.
    swapChainDetails.Format = device.getSurfaceFormats2KHR(physicalDeviceSurfaceInfo);    

    /// -- PRESENTATION MODES --
    swapChainDetails.presentationMode = device.getSurfacePresentModesKHR(this->surface);

    return swapChainDetails;
}

vk::SurfaceFormat2KHR VulkanRenderer::chooseBestSurfaceFormat(const std::vector<vk::SurfaceFormat2KHR > & formats) {
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    /// "Best" format is subjective.
    /// - Format        : vk::Format::eR8G8B8A8Unorm <-- RGBA, all 8 bits (??)
    /// -- vk::Format::eB8G8R8A8Unorm is also a possible option... used as 'backup'... (Will be used either if the other choice doesn't exist or this is listed before...
    /// - Color Space   : vk::ColorSpaceKHR::eVkColorspaceSrgbNonlinear

    /// If the only format in the list is undefined; This means ALL formats are supported!
    if (formats.size() == 1 && formats[0].surfaceFormat.format == vk::Format::eUndefined) {
        return vk::SurfaceFormat2KHR(vk::SurfaceFormatKHR({vk::Format::eR8G8B8A8Unorm,vk::ColorSpaceKHR::eVkColorspaceSrgbNonlinear}));
    }

    /// If some formats are unavailable, check if the requested formats exist (i.e. vk::Format::eR8G8B8A8Unorm)
    for (const auto &format : formats) {

        if ((format.surfaceFormat.format == vk::Format::eR8G8B8A8Unorm || format.surfaceFormat.format == vk::Format::eB8G8R8A8Unorm) &&
            format.surfaceFormat.colorSpace == vk::ColorSpaceKHR::eVkColorspaceSrgbNonlinear) 
        {
            return format;
        }
    }
    
    /// If no 'best format' is found, then we return the first format...This is however very unlikely
    return formats[0];
}

vk::PresentModeKHR VulkanRenderer::chooseBestPresentationMode(const std::vector<vk::PresentModeKHR> &presentationModes) {
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
return vk::PresentModeKHR::eImmediate;
    /// "Best" PresentationMode is subjective. Here we pick 'VSYNC'...
    std::vector<vk::PresentModeKHR> priorityList{ /// The different types we setttle with, first = highset priority
         vk::PresentModeKHR::eMailbox,
         vk::PresentModeKHR::eImmediate,
         vk::PresentModeKHR::eFifo
    }; 

    for(auto prioritized_mode : priorityList){

        if (std::find(presentationModes.begin(),presentationModes.end(), prioritized_mode) != presentationModes.end()){
            return prioritized_mode;
        }
    }

    /// If Mailbox Mode does not exist, use FIFO since it always should be available...
    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D VulkanRenderer::chooseBestImageResolution(const vk::SurfaceCapabilities2KHR &surfaceCapabilities) {
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    /// Since the extents width and height are represented with uint32_t values.
    /// Thus we want to make sure that the uin32_t value can represent the size of our surface (??).

    /// The default currentExtent.width value of a surface will be the size of the created window...
    /// - This is set by the glfwCreateWindowSurface function...
    /// - NOTE: the surfaces currentExtent.width/height can change, and then it will not be equal to the window size(??)
    if (surfaceCapabilities.surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        ///IF the current Extent does not vary, then the value will be the same as the windows currentExtent...
        /// - This will be the case if the currentExtent.width/height is NOT equal to the maximum value of a uint32_t...
        return surfaceCapabilities.surfaceCapabilities.currentExtent;
    } else {
        ///IF The current Extent vary, Then currentExtent.width/height will be set to the maximum size of a uint32_t!
        /// - This means that we do have to Define it ourself! i.e. grab the size from our glfw_window!
        int width=0, height=0;        
        //glfwGetFramebufferSize(this->window.glfw_window, &width, &height);
        SDL_GetWindowSize(this->window.sdl_window, &width, &height);

        /// Create a new extent using the current window size
        vk::Extent2D newExtent = {};
        newExtent.height = static_cast<uint32_t>(height);     /// glfw uses int, but VkExtent2D uses uint32_t...
        newExtent.width  = static_cast<uint32_t>(width);

        /// Make sure that height/width fetched from the glfw_window is within the max/min height/width of our surface
        /// - Do this by clamping the new height and width
        newExtent.width = std::clamp(newExtent.width,
                                     surfaceCapabilities.surfaceCapabilities.minImageExtent.width,
                                     surfaceCapabilities.surfaceCapabilities.maxImageExtent.width);
        newExtent.height = std::clamp(newExtent.height,
                                      surfaceCapabilities.surfaceCapabilities.minImageExtent.height,
                                      surfaceCapabilities.surfaceCapabilities.maxImageExtent.height);

        return newExtent;
    }

    /*! From what I understand:
     * We can either have a Window that is resizeable or static.
     * When a Window is static the surfaceCapabilities.currentExtent.width/height will be equal to the windows size!
     * When a Window is resizeable the surfaceCapabilities.currentExtent.width/height will have the maximum value of what a uint32_t can represent!
     * */

}

vk::Format VulkanRenderer::chooseSupportedFormat(const std::vector<vk::Format> &formats, vk::ImageTiling tiling, vk::FormatFeatureFlagBits featureFlags) const
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    bool is_optimal = false;
    bool is_linear = false;
    /// Loop through the options and pick the best suitable one...
  
    for(auto format : formats)
    {
        /// Get Properties for a given format on this device
        vk::FormatProperties2 properties = mainDevice.physicalDevice.getFormatProperties2(format);

        is_linear = (tiling == vk::ImageTiling::eLinear &&                                /// Checks whether tiling is set Linear
                    (properties.formatProperties.linearTilingFeatures & featureFlags) == featureFlags);  /// Checks if the device has the requested LinearTilingFeatures 

        is_optimal = (tiling == vk::ImageTiling::eOptimal &&                              /// Checks whether tiling is set Optimal
                    (properties.formatProperties.optimalTilingFeatures & featureFlags) == featureFlags); /// Checks if the device has the requested OptimalTilingFeatures

        /// Depending on choice of Tiling, check different features requirments
        if( is_linear || is_optimal)
        {
            return format;
        }
    }
    throw std::runtime_error("Failed to find a matching format!");
}

vk::Image VulkanRenderer::createImage(createImageData &&imageData, const std::string &imageDescription) 
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    /// - Create Image - 
    /// Image Createion Info
    vk::ImageCreateInfo imageCreateInfo;
    imageCreateInfo.setImageType(vk::ImageType::e2D);         /// We will use normal 2D images for everything... (could also use 1D and 3D)
    imageCreateInfo.setExtent(vk::Extent3D(
        imageData.width,    /// width of Image extent
        imageData.height,   /// height of Image extent
        1                   /// depth of Image (Just 1, means no 3D)
    ));
    imageCreateInfo.setMipLevels(uint32_t(1));                        /// number of mipmap levels 
    imageCreateInfo.setArrayLayers(uint32_t(1));                        /// number of Levels in image array
    imageCreateInfo.setFormat(imageData.format);                   /// Format of the image
    imageCreateInfo.setTiling(imageData.tiling);                   /// How image data should be "tiled" (arranged for optimal reading speed)
    imageCreateInfo.setInitialLayout(vk::ImageLayout::eUndefined);/// Layout of image data on creation
    imageCreateInfo.setUsage(imageData.useFlags);                 /// Flags defining how the image will be used
    imageCreateInfo.setSamples(vk::SampleCountFlagBits::e1);    /// Number of samples to be used for multi-sampling (1 since we dont use multisampling)
    imageCreateInfo.setSharingMode(vk::SharingMode::eExclusive);/// wheter the image will be shared between queues (we will not share images between queues...)

    VmaAllocationCreateInfo vmaAllocCreateInfo{}; 
    vmaAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    
    vk::Image image;    
    
    // /// Create the Image
    if(vmaCreateImage(this->vma,(VkImageCreateInfo*)&imageCreateInfo,&vmaAllocCreateInfo,(VkImage*)&image, imageData.imageMemory,nullptr) != VK_SUCCESS){
        throw std::runtime_error("Failed to Allocate Image through VMA!");
    }

    return image;
}

vk::ImageView VulkanRenderer::createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags) const
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    /// Creating a ImageView is similar to how we have the Physical Device and from that create a Logical Device...
    vk::ImageViewCreateInfo viewCreateInfo;

    viewCreateInfo.setImage(image);                           /// Image to create view for
    viewCreateInfo.setViewType(vk::ImageViewType::e2D);                           /// Type of Image (1D, 2D, 3D, Cube, etc)
    viewCreateInfo.setFormat(format);                             /// format of Image Data    
    viewCreateInfo.setComponents(vk::ComponentMapping( /// Allows remapping of rgba components to other rgba values!, Identity means they represent themselves
        vk::ComponentSwizzle::eIdentity,    // r
        vk::ComponentSwizzle::eIdentity,    // g
        vk::ComponentSwizzle::eIdentity,    // b
        vk::ComponentSwizzle::eIdentity     // a
    ));
    
    /*! Swizzling: Gives us a lot of flexibility on how we retrieve a value...
     * - Example:
     * -- pixelColor.r      <-- here r will describe how much red the pixel contains.
     * -- pixelColor.rg     <-- here rg will give us two values, the value of red and value of green...
     * -- pixelColor.rgrg   <-- here rgrg will give us the red value twice and also the green value twice...
     * */

    /// Subresources allow the view to view only a Part of a image!
    viewCreateInfo.subresourceRange.aspectMask      = aspectFlags;      /// Which aspect of image to view (e.g. COLOR_BIT for view)
    /*! Possible aspectMask values are defined with: is vk::ImageAspectFlagBits ...
     * - The 'regular' one is vk::ImageAspectFlagBits::eColor, used for images...
     * */
    viewCreateInfo.subresourceRange.baseMipLevel    = 0;                /// Which part of the image to view start view from, (a Image can have multiple Mip and Array Layers)...
    viewCreateInfo.subresourceRange.levelCount      = 1;                /// How many MipMap levels to view, we only view 1 and that will be the "0" referred to by baseMipLevel
    viewCreateInfo.subresourceRange.baseArrayLayer  = 0;                /// Which BaseArrayLayer to start from, we pick the first: 0
    viewCreateInfo.subresourceRange.layerCount      = 1;                /// How many layers to check from the baseArrayLayer... (i.e. only view the first layer, layer 0...)

    /// Create image view and Return it
    return  mainDevice.logicalDevice.createImageView(viewCreateInfo);;
}

void VulkanRenderer::createGraphicsPipeline_Base() 
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    /// read in SPIR-V code of shaders
    auto vertexShaderCode = vengine_helper::readShaderFile("shader.vert.spv");
    auto fragShaderCode = vengine_helper::readShaderFile("shader.frag.spv");

    /// Build Shader Modules to link to Graphics Pipeline
    /// Create Shader Modules
    vk::ShaderModule vertexShaderModule = this->createShaderModule(vertexShaderCode);
    vk::ShaderModule fragmentShaderModule = this->createShaderModule(fragShaderCode);
    registerVkObjectDbgInfo("ShaderModule VertexShader", vk::ObjectType::eShaderModule, reinterpret_cast<uint64_t>(vk::ShaderModule::CType(vertexShaderModule)));
    registerVkObjectDbgInfo("ShaderModule fragmentShader", vk::ObjectType::eShaderModule, reinterpret_cast<uint64_t>(vk::ShaderModule::CType(fragmentShaderModule)));

    /// --- SHADER STAGE CREATION INFORMATION ---
    /// Vertex Stage Creation Information
    vk::PipelineShaderStageCreateInfo vertexShaderStageCreateInfo {};
    vertexShaderStageCreateInfo.setStage(vk::ShaderStageFlagBits::eVertex);                             /// Shader stage name
    vertexShaderStageCreateInfo.setModule(vertexShaderModule);                                    /// Shader Modual to be used by Stage
    vertexShaderStageCreateInfo.setPName("main");                                                 ///Name of the vertex Shaders main function (function to run)

    /// Fragment Stage Creation Information
    vk::PipelineShaderStageCreateInfo fragmentShaderPipelineCreatInfo {};
    fragmentShaderPipelineCreatInfo.setStage(vk::ShaderStageFlagBits::eFragment);                       /// Shader Stage Name
    fragmentShaderPipelineCreatInfo.setModule(fragmentShaderModule);                              /// Shader Module used by stage
    fragmentShaderPipelineCreatInfo.setPName("main");                                             /// name of the fragment shader main function (function to run)
    
    /// Put shader stage creation infos into array
    /// graphics pipeline creation info requires array of shader stage creates
    std::array<vk::PipelineShaderStageCreateInfo, 2> pipelineShaderStageCreateInfos {
            vertexShaderStageCreateInfo,
            fragmentShaderPipelineCreatInfo
    };

    // -- FIXED SHADER STAGE CONFIGURATIONS -- 

    /// How the data for a single vertex (including info such as position, colour, texture coords, normals, etc)
    //// is as a whole.
    vk::VertexInputBindingDescription bindingDescription;
    bindingDescription.setBinding(uint32_t (0));                 /// Can bind multiple streams of data, this defines which one.
    bindingDescription.setStride(sizeof(Vertex));    /// Size of a single Vertex Object
    bindingDescription.setInputRate(vk::VertexInputRate::eVertex); /// How to move between data after each vertex...
                                /// vk::VertexInputRate::eVertex   : Move on to the next vertex 
                                /// vk::VertexInputRate::eInstance : Move to a vertex for the next instance

    /// How the Data  for an attribute is definied within a vertex    
    std::array<vk::VertexInputAttributeDescription, 3> attributeDescriptions{}; 

    /// Position Attribute: 
    attributeDescriptions[0].setBinding(uint32_t (0));                           /// which binding the data is at (should be same as above)
    attributeDescriptions[0].setLocation(uint32_t (0));                           /// Which Location in shader where data will be read from
    attributeDescriptions[0].setFormat(vk::Format::eR32G32B32Sfloat);  /// Format the data will take (also helps define size of data)
    attributeDescriptions[0].setOffset(offsetof(Vertex, pos));       /// Sets the offset of our struct member Pos (where this attribute is defined for a single vertex...)

    /// Color Attribute.
    attributeDescriptions[1].setBinding(uint32_t (0));                         
    attributeDescriptions[1].setLocation(uint32_t (1));                         
    attributeDescriptions[1].setFormat(vk::Format::eR32G32B32Sfloat);
    attributeDescriptions[1].setOffset(offsetof(Vertex, col));

    /// Texture Coorinate Attribute (uv): 
    attributeDescriptions[2].setBinding(uint32_t (0));
    attributeDescriptions[2].setLocation(uint32_t (2));
    attributeDescriptions[2].setFormat(vk::Format::eR32G32Sfloat);      /// Note; only RG, since it's a 2D image we don't use the depth and thus we only need RG and not RGB
    attributeDescriptions[2].setOffset(offsetof(Vertex, tex));


    /// -- VERTEX INPUT --
    vk::PipelineVertexInputStateCreateInfo vertexInputCreateInfo;
    vertexInputCreateInfo.setVertexBindingDescriptionCount(uint32_t (1));
    vertexInputCreateInfo.setPVertexBindingDescriptions(&bindingDescription);
    vertexInputCreateInfo.setVertexAttributeDescriptionCount(static_cast<uint32_t>(attributeDescriptions.size()));
    vertexInputCreateInfo.setPVertexAttributeDescriptions(attributeDescriptions.data());      

    /// -- INPUT ASSEMBLY --
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo;
    inputAssemblyStateCreateInfo.setTopology(vk::PrimitiveTopology::eTriangleList);        /// Primitive type to assemble primitives from ;
    inputAssemblyStateCreateInfo.setPrimitiveRestartEnable(VK_FALSE);                     /// Allow overrideing tof "strip" topology to start new primitives

    /// -- VIEWPORT & SCISSOR ---
    /// Create viewport info struct
    vk::Viewport viewport;    
    viewport.setX(0.0F);        /// x start coordinate
    viewport.setY(0.0F);        /// y start coordinate
    viewport.setWidth(static_cast<float>(swapChainExtent.width));       /// width of viewport
    viewport.setHeight(static_cast<float>(swapChainExtent.height));     /// height of viewport
    viewport.setMinDepth(0.0F);     /// min framebuffer depth
    viewport.setMaxDepth(1.0F);     /// max framebuffer depth
    ///create a scissor info struct
    vk::Rect2D scissor;
    scissor.offset = vk::Offset2D{0, 0};                            /// Offset to use Region from
    scissor.extent = swapChainExtent;                   /// Extent to sdescribe region to use, starting at offset

    vk::PipelineViewportStateCreateInfo viewportStateCreateInfo = {};
    viewportStateCreateInfo.setViewportCount(uint32_t(1));
    viewportStateCreateInfo.setPViewports(&viewport);
    viewportStateCreateInfo.setScissorCount(uint32_t(1));
    viewportStateCreateInfo.setPScissors(&scissor);


    /// --- DYNAMIC STATES ---
    /// dynamic states to enable... NOTE: to change some stuff you might need to reset it first... example viewport...
    std::vector<vk::DynamicState > dynamicStateEnables;
    dynamicStateEnables.push_back(vk::DynamicState::eViewport); /// Dynamic Viewport    : can resize in command buffer with vkCmdSetViewport(command, 0,1,&viewport)
    dynamicStateEnables.push_back(vk::DynamicState::eScissor);  /// Dynamic Scissor     : Can resize in commandbuffer with vkCmdSetScissor(command, 0,1, &viewport)

    /// Dynamic state creation info
    vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};    
    dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
    dynamicStateCreateInfo.pDynamicStates = dynamicStateEnables.data();

    /// --- RASTERIZER ---
    vk::PipelineRasterizationStateCreateInfo rasterizationStateCreateInfo;
    rasterizationStateCreateInfo.setDepthClampEnable(VK_FALSE);         /// Change if fragments beyond near/far planes are clipped / clamped  (Requires depthclamp as a device features...)
    rasterizationStateCreateInfo.setRasterizerDiscardEnable(VK_FALSE);/// Wether to discard data or skip rasterizzer. Never creates fragments, only suitable for pipleine without framebuffer output
    rasterizationStateCreateInfo.setPolygonMode(vk::PolygonMode::eFill);/// How to handle filling point betweeen vertices...
    rasterizationStateCreateInfo.setLineWidth(1.F);                  /// how thiock lines should be when drawn (other than 1 requires device features...)
    rasterizationStateCreateInfo.setCullMode(vk::CullModeFlagBits::eBack);  /// Which face of tri to cull
    rasterizationStateCreateInfo.setFrontFace(vk::FrontFace::eCounterClockwise);/// Since our Projection matrix now has a inverted Y-axis (GLM is right handed, but vulkan is left handed)
                                                                    //// winding order to determine which side is front
    rasterizationStateCreateInfo.setDepthBiasEnable(VK_FALSE);        /// Wether to add depthbiaoas to fragments (to remove shadowacne...)

    /// --- MULTISAMPLING ---
    vk::PipelineMultisampleStateCreateInfo multisampleStateCreateInfo;
    multisampleStateCreateInfo.setSampleShadingEnable(VK_FALSE);                      /// Enable multisample shading or not
    multisampleStateCreateInfo.setRasterizationSamples(vk::SampleCountFlagBits::e1);        /// number of samples to use per fragment


    /// --- BLENDING --
    ///Blending decides how to blend ea new colour being written to a fragment, with the old value

    /// Blend attatchment State (how blending is handled)
    vk::PipelineColorBlendAttachmentState  colorState;
    colorState.setColorWriteMask(vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eB /// Colours to apply blending to
                | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eR);
    colorState.setBlendEnable(VK_TRUE);                                                       /// enable blending

    /// Blending uses Equation: (srcColorBlendFactor * new Colour) colorBlendOp (dstColorBlendFactor * old Colour)
    colorState.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha);
    colorState.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha);
    colorState.setColorBlendOp(vk::BlendOp::eAdd);

    /// summarised (VK_BLEND_FACTOR_SRC_ALPHA * new Colour) + (VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA * old colour)
    ///             (new colour alpha * new colour) + ((1- new colour alpha) * old colour)

    colorState.setSrcAlphaBlendFactor(vk::BlendFactor::eOne);
    colorState.setDstAlphaBlendFactor(vk::BlendFactor::eZero);
    colorState.setAlphaBlendOp(vk::BlendOp::eAdd);

    /// summarised : (1 * new alpha ) +  (0 * old Alpha ) = new alpha

    

    vk::PipelineColorBlendStateCreateInfo colorBlendingCreateInfo;
    colorBlendingCreateInfo.setLogicOpEnable(VK_FALSE);               /// Alternative to calculations is to use logical Operations
    colorBlendingCreateInfo.setAttachmentCount(uint32_t (1));
    colorBlendingCreateInfo.setPAttachments(&colorState);

    /// --- PIPELINE LAYOUT ---

    /// We have two Descriptor Set Layouts, 
    //// One for View and Projection matrices Uniform Buffer, and the other one for the texture sampler!
    std::array<vk::DescriptorSetLayout, 2> descriptorSetLayouts{
        this->descriptorSetLayout,
        this->samplerDescriptorSetLayout
    };

    vk::PipelineLayoutCreateInfo  pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.setSetLayoutCount(static_cast<uint32_t>(descriptorSetLayouts.size()));
    pipelineLayoutCreateInfo.setPSetLayouts(descriptorSetLayouts.data());
    pipelineLayoutCreateInfo.setPushConstantRangeCount(uint32_t(1));                    /// We only have One Push Constant range we want to use
    pipelineLayoutCreateInfo.setPPushConstantRanges(&this->pushConstantRange);/// the Push Constant Range we want to use

    /// --- DEPTH STENCIL TESING ---
    vk::PipelineDepthStencilStateCreateInfo depthStencilCreateInfo{};
    depthStencilCreateInfo.setDepthTestEnable(VK_TRUE);              /// Enable Depth Testing; Check the Depth to determine if it should write to the fragment
    depthStencilCreateInfo.setDepthWriteEnable(VK_TRUE);              /// enable writing to Depth Buffer; To make sure it replaces old values
    depthStencilCreateInfo.setDepthCompareOp(vk::CompareOp::eLess);   /// Describes that we want to replace the old values IF new values are smaller/lesser.
    depthStencilCreateInfo.setDepthBoundsTestEnable(VK_FALSE);             /// In case we want to use as Min and a Max Depth; if depth Values exist between two bounds... 
    depthStencilCreateInfo.setStencilTestEnable(VK_FALSE);             /// Whether to enable the Stencil Test; we dont use it so we let it be disabled

    this->pipelineLayout = this->mainDevice.logicalDevice.createPipelineLayout(pipelineLayoutCreateInfo);
    registerVkObjectDbgInfo("VkPipelineLayout GraphicsPipelineLayout", vk::ObjectType::ePipelineLayout, reinterpret_cast<uint64_t>(vk::PipelineLayout::CType(this->pipelineLayout)));
    /// --- RENDER PASS ---
    //createRenderPass(); <-- This is done in the init-function!

    /// -- GRAPHICS PIPELINE CREATION --
    vk::GraphicsPipelineCreateInfo pipelineCreateInfo;
    pipelineCreateInfo.setStageCount(uint32_t(2));                                          /// Number of shader stages
    pipelineCreateInfo.setPStages(pipelineShaderStageCreateInfos.data());         /// List of Shader stages
    pipelineCreateInfo.setPVertexInputState(&vertexInputCreateInfo);              /// All the fixed function Pipeline states
    pipelineCreateInfo.setPInputAssemblyState(&inputAssemblyStateCreateInfo);
    pipelineCreateInfo.setPViewportState(&viewportStateCreateInfo);
    pipelineCreateInfo.setPDynamicState(&dynamicStateCreateInfo);
    pipelineCreateInfo.setPRasterizationState(&rasterizationStateCreateInfo);
    pipelineCreateInfo.setPMultisampleState(&multisampleStateCreateInfo);
    pipelineCreateInfo.setPColorBlendState(&colorBlendingCreateInfo);
    pipelineCreateInfo.setPDepthStencilState(&depthStencilCreateInfo);
    pipelineCreateInfo.setLayout(this->pipelineLayout);                                 /// Pipeline layout pipeline should use
    pipelineCreateInfo.setRenderPass(this->renderPass_base);                                 /// Render pass description the pipeline is compatible with
    pipelineCreateInfo.setSubpass(uint32_t(0));                                             /// subpass of render pass to use with pipeline

    /// Pipeline Derivatives : Can Create multiple pipelines that derive from one another for optimization
    pipelineCreateInfo.setBasePipelineHandle(nullptr); /// Existing pipeline to derive from ...
    pipelineCreateInfo.setBasePipelineIndex(int32_t(-1));              /// or index of pipeline being created to derive from ( in case of creating multiple of at once )

    /// Create Graphics Pipeline
    vk::Result result{};
    std::tie(result , this->graphicsPipeline) = mainDevice.logicalDevice.createGraphicsPipeline(nullptr,pipelineCreateInfo);
    if(result != vk::Result::eSuccess){throw std::runtime_error("Could not create Pipeline");}
    registerVkObjectDbgInfo("VkPipeline GraphicsPipeline", vk::ObjectType::ePipeline, reinterpret_cast<uint64_t>(vk::Pipeline::CType(this->graphicsPipeline)));

    ///Destroy Shader Moduels, no longer needed after pipeline created
    this->mainDevice.logicalDevice.destroyShaderModule(vertexShaderModule);
    this->mainDevice.logicalDevice.destroyShaderModule(fragmentShaderModule);


    ////////////////////////////////////////////////
    ////////////////////////////////////////////////
    ////////////////////////////////////////////////
    ////////////////////////////////////////////////
    
    /// - CREATE SECOND PASS PIPELINE - 
    /// second Pass Shaders
    auto secondVertexShaderCode     = vengine_helper::readShaderFile("second.vert.spv");
    auto secondFragmentShaderCode     = vengine_helper::readShaderFile("second.frag.spv");    

    /// Build Shaders
    vk::ShaderModule secondVertexShaderModule = createShaderModule(secondVertexShaderCode);
    vk::ShaderModule secondFragmentShaderModule = createShaderModule(secondFragmentShaderCode);
    registerVkObjectDbgInfo("ShaderModule Second_VertexShader", vk::ObjectType::eShaderModule, reinterpret_cast<uint64_t>(vk::ShaderModule::CType(secondVertexShaderModule)));
    registerVkObjectDbgInfo("ShaderModule Second_fragmentShader", vk::ObjectType::eShaderModule, reinterpret_cast<uint64_t>(vk::ShaderModule::CType(secondFragmentShaderModule)));

    /// --- SHADER STAGE CREATION INFORMATION ---
    /// Vertex Stage Creation Information
    vertexShaderStageCreateInfo.module = secondVertexShaderModule;                              /// Shader Modual to be used by Stage

    /// Fragment Stage Creation Information    
    fragmentShaderPipelineCreatInfo.module = secondFragmentShaderModule;                        /// Shader Module used by stage    

    /// Put shader stage creation infos into array
    /// graphics pipeline creation info requires array of shader stage creates
    std::array<vk::PipelineShaderStageCreateInfo,2> secondPipelineShaderStageCreateInfos = {
            vertexShaderStageCreateInfo,
            fragmentShaderPipelineCreatInfo
    };
    
     /// -- VERTEX INPUT --
    vertexInputCreateInfo.setVertexBindingDescriptionCount(uint32_t(0));
    vertexInputCreateInfo.setPVertexBindingDescriptions(nullptr);
    vertexInputCreateInfo.setVertexAttributeDescriptionCount(uint32_t (0));
    vertexInputCreateInfo.setPVertexAttributeDescriptions(nullptr);  

    /// --- DEPTH STENCIL TESING ---
    depthStencilCreateInfo.setDepthWriteEnable(VK_FALSE);             /// DISABLE writing to Depth Buffer; To make sure it replaces old values

    /// Create New pipeline Layout
    vk::PipelineLayoutCreateInfo secondPipelineLayoutCreateInfo{};
    secondPipelineLayoutCreateInfo.setSetLayoutCount(uint32_t (1));
    secondPipelineLayoutCreateInfo.setPSetLayouts(&this->inputSetLayout);
    secondPipelineLayoutCreateInfo.setPushConstantRangeCount(uint32_t (0));
    secondPipelineLayoutCreateInfo.setPPushConstantRanges(nullptr);
    /// createPipelineLayout
    this->secondPipelineLayout = this->mainDevice.logicalDevice.createPipelineLayout(secondPipelineLayoutCreateInfo);
    registerVkObjectDbgInfo("VkPipelineLayout Second_GraphicsPipelineLayout", vk::ObjectType::ePipelineLayout, reinterpret_cast<uint64_t>(vk::PipelineLayout::CType(this->secondPipelineLayout)));
    
     /// -- GRAPHICS PIPELINE CREATION --
    pipelineCreateInfo.setPStages(secondPipelineShaderStageCreateInfos.data());                /// List of Shader stages
    pipelineCreateInfo.setLayout(this->secondPipelineLayout);                      /// Pipeline layout pipeline should use
    pipelineCreateInfo.setSubpass(uint32_t (1));                                             /// Use Subpass 2 (index 1...)

    /// Create Graphics Pipeline
    std::tie(result, this->secondGraphicsPipeline) = this->mainDevice.logicalDevice.createGraphicsPipeline(nullptr, pipelineCreateInfo);
    if(result != vk::Result::eSuccess){throw std::runtime_error("Could not create Second Pipeline");}
    registerVkObjectDbgInfo("VkPipeline Second_GraphicsPipeline", vk::ObjectType::ePipeline, reinterpret_cast<uint64_t>(vk::Pipeline::CType(this->secondGraphicsPipeline)));

    ///Destroy Second Shader Moduels, no longer needed after pipeline created
    this->mainDevice.logicalDevice.destroyShaderModule(secondVertexShaderModule);
    this->mainDevice.logicalDevice.destroyShaderModule(secondFragmentShaderModule);

}

void VulkanRenderer::createGraphicsPipeline_Imgui()
{
    
}

void VulkanRenderer::createGraphicsPipeline_DynamicRendering() //NOLINT: 
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    static const char* pipeline_cache_name = "pipeline_cache.data";
        /// Pipeline Cache

    try
	{   
         std::vector<char> pipeline_data;
		pipeline_data = vengine_helper::readFile(pipeline_cache_name);

        uint32_t headerLength = 0;
        uint32_t cacheHeaderVersion = 0;
        uint32_t vendorID = 0;
        uint32_t deviceID = 0;
        uint8_t pipelineCacheUUID[VK_UUID_SIZE] = {};

        memcpy(&headerLength, (uint8_t *)pipeline_data.data() + 0, 4);
        memcpy(&cacheHeaderVersion, (uint8_t *)pipeline_data.data() + 4, 4);
        memcpy(&vendorID, (uint8_t *)pipeline_data.data() + 8, 4);
        memcpy(&deviceID, (uint8_t *)pipeline_data.data() + 12, 4);
        memcpy(pipelineCacheUUID, (uint8_t *)pipeline_data.data() + 16, VK_UUID_SIZE);

        vk::PipelineCacheCreateInfo pipelineCacheCreateInfo{};
        pipelineCacheCreateInfo.setInitialDataSize(pipeline_data.size());
        pipelineCacheCreateInfo.setPInitialData((uint8_t *)pipeline_data.data());
        if(this->mainDevice.logicalDevice.createPipelineCache(&pipelineCacheCreateInfo, nullptr, &graphics_pipelineCache) != vk::Result::eSuccess){
            throw std::runtime_error("Failed to create PipelineCache");
        }
	}
    catch(std::exception &e){
        std::cout << "Could not load "<<pipeline_cache_name<<" from disk...\n";

        vk::PipelineCacheCreateInfo pipelineCacheCreateInfo{};
        pipelineCacheCreateInfo.setInitialDataSize(0);
        pipelineCacheCreateInfo.setPInitialData(nullptr);
        if(this->mainDevice.logicalDevice.createPipelineCache(&pipelineCacheCreateInfo, nullptr, &graphics_pipelineCache) != vk::Result::eSuccess){
            throw std::runtime_error("Failed to create PipelineCache");
        }
    }

	{
    /// read in SPIR-V code of shaders
    auto vertexShaderCode = vengine_helper::readShaderFile("shader.vert.spv");
    auto fragShaderCode = vengine_helper::readShaderFile("shader.frag.spv");

    /// Build Shader Modules to link to Graphics Pipeline
    /// Create Shader Modules
    vk::ShaderModule vertexShaderModule = this->createShaderModule(vertexShaderCode);
    vk::ShaderModule fragmentShaderModule = this->createShaderModule(fragShaderCode);
    registerVkObjectDbgInfo("ShaderModule VertexShader", vk::ObjectType::eShaderModule, reinterpret_cast<uint64_t>(vk::ShaderModule::CType(vertexShaderModule)));
    registerVkObjectDbgInfo("ShaderModule fragmentShader", vk::ObjectType::eShaderModule, reinterpret_cast<uint64_t>(vk::ShaderModule::CType(fragmentShaderModule)));

    /// --- SHADER STAGE CREATION INFORMATION ---
    /// Vertex Stage Creation Information
    vk::PipelineShaderStageCreateInfo vertexShaderStageCreateInfo {};
    vertexShaderStageCreateInfo.setStage(vk::ShaderStageFlagBits::eVertex);                             /// Shader stage name
    vertexShaderStageCreateInfo.setModule(vertexShaderModule);                                    /// Shader Modual to be used by Stage
    vertexShaderStageCreateInfo.setPName("main");                                                 ///Name of the vertex Shaders main function (function to run)

    /// Fragment Stage Creation Information
    vk::PipelineShaderStageCreateInfo fragmentShaderPipelineCreatInfo {};
    fragmentShaderPipelineCreatInfo.setStage(vk::ShaderStageFlagBits::eFragment);                       /// Shader Stage Name
    fragmentShaderPipelineCreatInfo.setModule(fragmentShaderModule);                              /// Shader Module used by stage
    fragmentShaderPipelineCreatInfo.setPName("main");                                             /// name of the fragment shader main function (function to run)
    
    /// Put shader stage creation infos into array
    /// graphics pipeline creation info requires array of shader stage creates
    std::array<vk::PipelineShaderStageCreateInfo, 2> pipelineShaderStageCreateInfos {
            vertexShaderStageCreateInfo,
            fragmentShaderPipelineCreatInfo
    };

    // -- FIXED SHADER STAGE CONFIGURATIONS -- 

    /// How the data for a single vertex (including info such as position, colour, texture coords, normals, etc)
    //// is as a whole.
    vk::VertexInputBindingDescription bindingDescription;
    bindingDescription.setBinding(uint32_t (0));                 /// Can bind multiple streams of data, this defines which one.
    bindingDescription.setStride(sizeof(Vertex));    /// Size of a single Vertex Object
    bindingDescription.setInputRate(vk::VertexInputRate::eVertex); /// How to move between data after each vertex...
                                /// vk::VertexInputRate::eVertex   : Move on to the next vertex 
                                /// vk::VertexInputRate::eInstance : Move to a vertex for the next instance

    /// How the Data  for an attribute is definied within a vertex    
    std::array<vk::VertexInputAttributeDescription, 3> attributeDescriptions{}; 

    /// Position Attribute: 
    attributeDescriptions[0].setBinding(uint32_t (0));                           /// which binding the data is at (should be same as above)
    attributeDescriptions[0].setLocation(uint32_t (0));                           /// Which Location in shader where data will be read from
    attributeDescriptions[0].setFormat(vk::Format::eR32G32B32Sfloat);  /// Format the data will take (also helps define size of data)
    attributeDescriptions[0].setOffset(offsetof(Vertex, pos));       /// Sets the offset of our struct member Pos (where this attribute is defined for a single vertex...)

    /// Color Attribute.
    attributeDescriptions[1].setBinding(uint32_t (0));                         
    attributeDescriptions[1].setLocation(uint32_t (1));                         
    attributeDescriptions[1].setFormat(vk::Format::eR32G32B32Sfloat);
    attributeDescriptions[1].setOffset(offsetof(Vertex, col));

    /// Texture Coorinate Attribute (uv): 
    attributeDescriptions[2].setBinding(uint32_t (0));
    attributeDescriptions[2].setLocation(uint32_t (2));
    attributeDescriptions[2].setFormat(vk::Format::eR32G32Sfloat);      /// Note; only RG, since it's a 2D image we don't use the depth and thus we only need RG and not RGB
    attributeDescriptions[2].setOffset(offsetof(Vertex, tex));


    /// -- VERTEX INPUT --
    vk::PipelineVertexInputStateCreateInfo vertexInputCreateInfo;
    vertexInputCreateInfo.setVertexBindingDescriptionCount(uint32_t (1));
    vertexInputCreateInfo.setPVertexBindingDescriptions(&bindingDescription);
    vertexInputCreateInfo.setVertexAttributeDescriptionCount(static_cast<uint32_t>(attributeDescriptions.size()));
    vertexInputCreateInfo.setPVertexAttributeDescriptions(attributeDescriptions.data());      

    /// -- INPUT ASSEMBLY --
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo;
    inputAssemblyStateCreateInfo.setTopology(vk::PrimitiveTopology::eTriangleList);        /// Primitive type to assemble primitives from ;
    inputAssemblyStateCreateInfo.setPrimitiveRestartEnable(VK_FALSE);                     /// Allow overrideing tof "strip" topology to start new primitives

    /// -- VIEWPORT & SCISSOR ---
    /// Create viewport info struct
    vk::Viewport viewport;    
    viewport.setX(0.0F);        /// x start coordinate
    viewport.setY(0.0F);        /// y start coordinate
    viewport.setWidth(static_cast<float>(swapChainExtent.width));       /// width of viewport
    viewport.setHeight(static_cast<float>(swapChainExtent.height));     /// height of viewport
    viewport.setMinDepth(0.0F);     /// min framebuffer depth
    viewport.setMaxDepth(1.0F);     /// max framebuffer depth
    ///create a scissor info struct
    vk::Rect2D scissor;
    scissor.offset = vk::Offset2D{0, 0};                            /// Offset to use Region from
    scissor.extent = swapChainExtent;                   /// Extent to sdescribe region to use, starting at offset

    vk::PipelineViewportStateCreateInfo viewportStateCreateInfo = {};
    viewportStateCreateInfo.setViewportCount(uint32_t(1));
    viewportStateCreateInfo.setPViewports(&viewport);
    viewportStateCreateInfo.setScissorCount(uint32_t(1));
    viewportStateCreateInfo.setPScissors(&scissor);


    /// --- DYNAMIC STATES ---
    /// dynamic states to enable... NOTE: to change some stuff you might need to reset it first... example viewport...
    /*std::vector<VkDynamicState > dynamicStateEnables;
    dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT); /// Dynamic Viewport    : can resize in command buffer with vkCmdSetViewport(command, 0,1,&viewport)
    dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);  /// Dynamic Scissor     : Can resize in commandbuffer with vkCmdSetScissor(command, 0,1, &viewport)

    /// Dynamic state creation info
    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
    dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
    dynamicStateCreateInfo.pDynamicStates = dynamicStateEnables.data();*/

    /// --- RASTERIZER ---
    vk::PipelineRasterizationStateCreateInfo rasterizationStateCreateInfo;
    rasterizationStateCreateInfo.setDepthClampEnable(VK_FALSE);         /// Change if fragments beyond near/far planes are clipped / clamped  (Requires depthclamp as a device features...)
    rasterizationStateCreateInfo.setRasterizerDiscardEnable(VK_FALSE);/// Wether to discard data or skip rasterizzer. Never creates fragments, only suitable for pipleine without framebuffer output
    rasterizationStateCreateInfo.setPolygonMode(vk::PolygonMode::eFill);/// How to handle filling point betweeen vertices...
    rasterizationStateCreateInfo.setLineWidth(1.F);                  /// how thiock lines should be when drawn (other than 1 requires device features...)
    rasterizationStateCreateInfo.setCullMode(vk::CullModeFlagBits::eBack);  /// Which face of tri to cull
    rasterizationStateCreateInfo.setFrontFace(vk::FrontFace::eCounterClockwise);/// Since our Projection matrix now has a inverted Y-axis (GLM is right handed, but vulkan is left handed)
                                                                    //// winding order to determine which side is front
    rasterizationStateCreateInfo.setDepthBiasEnable(VK_FALSE);        /// Wether to add depthbiaoas to fragments (to remove shadowacne...)

    /// --- MULTISAMPLING ---
    vk::PipelineMultisampleStateCreateInfo multisampleStateCreateInfo;
    multisampleStateCreateInfo.setSampleShadingEnable(VK_FALSE);                      /// Enable multisample shading or not
    multisampleStateCreateInfo.setRasterizationSamples(vk::SampleCountFlagBits::e1);        /// number of samples to use per fragment


    /// --- BLENDING --
    ///Blending decides how to blend ea new colour being written to a fragment, with the old value

    /// Blend attatchment State (how blending is handled)
    vk::PipelineColorBlendAttachmentState  colorState;
    colorState.setColorWriteMask(vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eB /// Colours to apply blending to
                | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eR);
    colorState.setBlendEnable(VK_TRUE);                                                       /// enable blending

    /// Blending uses Equation: (srcColorBlendFactor * new Colour) colorBlendOp (dstColorBlendFactor * old Colour)
    colorState.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha);
    colorState.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha);
    colorState.setColorBlendOp(vk::BlendOp::eAdd);

    /// summarised (VK_BLEND_FACTOR_SRC_ALPHA * new Colour) + (VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA * old colour)
    ///             (new colour alpha * new colour) + ((1- new colour alpha) * old colour)

    colorState.setSrcAlphaBlendFactor(vk::BlendFactor::eOne);
    colorState.setDstAlphaBlendFactor(vk::BlendFactor::eZero);
    colorState.setAlphaBlendOp(vk::BlendOp::eAdd);

    /// summarised : (1 * new alpha ) +  (0 * old Alpha ) = new alpha

    

    vk::PipelineColorBlendStateCreateInfo colorBlendingCreateInfo;
    colorBlendingCreateInfo.setLogicOpEnable(VK_FALSE);               /// Alternative to calculations is to use logical Operations
    colorBlendingCreateInfo.setAttachmentCount(uint32_t (1));
    colorBlendingCreateInfo.setPAttachments(&colorState);

    /// --- PIPELINE LAYOUT ---

    /// We have two Descriptor Set Layouts, 
    //// One for View and Projection matrices Uniform Buffer, and the other one for the texture sampler!
    std::array<vk::DescriptorSetLayout, 2> descriptorSetLayouts{
        this->descriptorSetLayout,
        this->samplerDescriptorSetLayout
    };

    vk::PipelineLayoutCreateInfo  pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.setSetLayoutCount(static_cast<uint32_t>(descriptorSetLayouts.size()));
    pipelineLayoutCreateInfo.setPSetLayouts(descriptorSetLayouts.data());
    pipelineLayoutCreateInfo.setPushConstantRangeCount(uint32_t(1));                    /// We only have One Push Constant range we want to use
    pipelineLayoutCreateInfo.setPPushConstantRanges(&this->pushConstantRange);/// the Push Constant Range we want to use

    /// --- DEPTH STENCIL TESING ---
    vk::PipelineDepthStencilStateCreateInfo depthStencilCreateInfo{};
    depthStencilCreateInfo.setDepthTestEnable(VK_TRUE);              /// Enable Depth Testing; Check the Depth to determine if it should write to the fragment
    depthStencilCreateInfo.setDepthWriteEnable(VK_TRUE);              /// enable writing to Depth Buffer; To make sure it replaces old values
    depthStencilCreateInfo.setDepthCompareOp(vk::CompareOp::eLess);   /// Describes that we want to replace the old values IF new values are smaller/lesser.
    depthStencilCreateInfo.setDepthBoundsTestEnable(VK_FALSE);             /// In case we want to use as Min and a Max Depth; if depth Values exist between two bounds... 
    depthStencilCreateInfo.setStencilTestEnable(VK_FALSE);             /// Whether to enable the Stencil Test; we dont use it so we let it be disabled

    this->pipelineLayout = this->mainDevice.logicalDevice.createPipelineLayout(pipelineLayoutCreateInfo);
    registerVkObjectDbgInfo("VkPipelineLayout GraphicsPipelineLayout", vk::ObjectType::ePipelineLayout, reinterpret_cast<uint64_t>(vk::PipelineLayout::CType(this->pipelineLayout)));
    /// --- Dynamic Rendering ---
    vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo{};
    pipelineRenderingCreateInfo.setColorAttachmentCount(uint32_t (1));
    pipelineRenderingCreateInfo.setColorAttachmentFormats(this->swapChainImageFormat);
    pipelineRenderingCreateInfo.setDepthAttachmentFormat(this->depthFormat);
    pipelineRenderingCreateInfo.setStencilAttachmentFormat(this->depthFormat);

    /// -- GRAPHICS PIPELINE CREATION --
    vk::GraphicsPipelineCreateInfo pipelineCreateInfo;
    pipelineCreateInfo.setStageCount(uint32_t(2));                                          /// Number of shader stages
    pipelineCreateInfo.setPStages(pipelineShaderStageCreateInfos.data());         /// List of Shader stages
    pipelineCreateInfo.setPVertexInputState(&vertexInputCreateInfo);              /// All the fixed function Pipeline states
    pipelineCreateInfo.setPInputAssemblyState(&inputAssemblyStateCreateInfo);
    pipelineCreateInfo.setPViewportState(&viewportStateCreateInfo);
    pipelineCreateInfo.setPDynamicState(nullptr);
    pipelineCreateInfo.setPRasterizationState(&rasterizationStateCreateInfo);
    pipelineCreateInfo.setPMultisampleState(&multisampleStateCreateInfo);
    pipelineCreateInfo.setPColorBlendState(&colorBlendingCreateInfo);
    pipelineCreateInfo.setPDepthStencilState(&depthStencilCreateInfo);
    pipelineCreateInfo.setLayout(this->pipelineLayout);                                 /// Pipeline layout pipeline should use
    pipelineCreateInfo.setPNext(&pipelineRenderingCreateInfo);

    pipelineCreateInfo.setRenderPass(nullptr);                                 /// Render pass description the pipeline is compatible with
    pipelineCreateInfo.setSubpass(uint32_t(0));                                             /// subpass of render pass to use with pipeline

    /// Pipeline Derivatives : Can Create multiple pipelines that derive from one another for optimization
    pipelineCreateInfo.setBasePipelineHandle(nullptr); /// Existing pipeline to derive from ...
    pipelineCreateInfo.setBasePipelineIndex(int32_t(-1));              /// or index of pipeline being created to derive from ( in case of creating multiple of at once )

    /// Create Graphics Pipeline
    vk::Result result{};
    std::tie(result , this->graphicsPipeline) = mainDevice.logicalDevice.createGraphicsPipeline(graphics_pipelineCache,pipelineCreateInfo);  
    if(result != vk::Result::eSuccess){throw std::runtime_error("Could not create Pipeline");}
    registerVkObjectDbgInfo("VkPipeline GraphicsPipeline", vk::ObjectType::ePipeline, reinterpret_cast<uint64_t>(vk::Pipeline::CType(this->graphicsPipeline)));

    /// Check cache header to validate if cache is ok
    uint32_t headerLength = 0;
    uint32_t cacheHeaderVersion = 0;
    uint32_t vendorID = 0;
    uint32_t deviceID = 0;
    uint8_t pipelineCacheUUID[VK_UUID_SIZE] = {};

    std::vector<uint8_t> loaded_cache;
    loaded_cache = this->mainDevice.logicalDevice.getPipelineCacheData(graphics_pipelineCache);

    memcpy(&headerLength, (uint8_t *)loaded_cache.data() + 0, 4);
    memcpy(&cacheHeaderVersion, (uint8_t *)loaded_cache.data() + 4, 4);
    memcpy(&vendorID, (uint8_t *)loaded_cache.data() + 8, 4);
    memcpy(&deviceID, (uint8_t *)loaded_cache.data() + 12, 4);
    memcpy(pipelineCacheUUID, (uint8_t *)loaded_cache.data() + 16, VK_UUID_SIZE);

    std::ofstream save_cache(pipeline_cache_name, std::ios::binary);

    save_cache.write((char*)loaded_cache.data(), static_cast<uint32_t>(loaded_cache.size()));
    save_cache.close();

    ///Destroy Shader Moduels, no longer needed after pipeline created
    this->mainDevice.logicalDevice.destroyShaderModule(vertexShaderModule);
    this->mainDevice.logicalDevice.destroyShaderModule(fragmentShaderModule);
    }
}

void VulkanRenderer::createColorBufferImage_Base()
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    this->colorBufferImage.resize(swapChainImages.size());
    this->colorBufferImageMemory.resize(swapChainImages.size());
    this->colorBufferImageView.resize(swapChainImages.size());

    /// Get supported formats for Color Attachment
    const std::vector<vk::Format> formats {vk::Format::eR8G8B8A8Unorm};
    this->colorFormat = chooseSupportedFormat(
        formats,
        vk::ImageTiling::eOptimal, 
        vk::FormatFeatureFlagBits::eColorAttachment);        

    for(size_t i = 0; i < this->swapChainImages.size(); i++)
    {
        colorBufferImage[i] = createImage(
            {
                .width = this->swapChainExtent.width,
                .height = this->swapChainExtent.height,
                .format = this->colorFormat,
                .tiling = vk::ImageTiling::eOptimal,
                .useFlags = vk::ImageUsageFlagBits::eColorAttachment     /// Image will be used as a Color Attachment
                            |   vk::ImageUsageFlagBits::eInputAttachment, /// This will be an Input Attachemnt, recieved by a subpass
                                                        //// TODO:; Not optimal for normal offscreen rendering, since we can only handle info for a current pixel/fragment
                .property = vk::MemoryPropertyFlagBits::eDeviceLocal,     /// Image will only be handled by the GPU
                .imageMemory = &this->colorBufferImageMemory[i]
            },
            "ColorBufferImage"
        );

        /// Creation of Color Buffer Image View
        this->colorBufferImageView[i] = createImageView(this->colorBufferImage[i], this->colorFormat, vk::ImageAspectFlagBits::eColor);
        registerVkObjectDbgInfo("colorBufferImageView["+std::to_string(i)+"]", vk::ObjectType::eImageView, reinterpret_cast<uint64_t>(vk::ImageView::CType(this->colorBufferImageView[i])));
        registerVkObjectDbgInfo("colorBufferImage["+std::to_string(i)+"]", vk::ObjectType::eImage, reinterpret_cast<uint64_t>(vk::Image::CType(this->colorBufferImage[i])));
    }

}

void VulkanRenderer::createDepthBufferImage()
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    this->depthBufferImage.resize(swapChainImages.size());
    this->depthBufferImageMemory.resize(swapChainImages.size());
    this->depthBufferImageView.resize(swapChainImages.size());


    /// Get supported VkFormat for the DepthBuffer
    this->depthFormat = chooseSupportedFormat(
        {  //Atleast one of these should be available...
            vk::Format::eD32SfloatS8Uint,   /// First  Choice: Supported format should be using depth value of 32 bits and using StencilBuffer 8Bits (??)
            vk::Format::eD32Sfloat,           /// Second Choice: Supported format shoudl be using depth value of 32 bits
            vk::Format::eD24UnormS8Uint     /// third  Choice: Supported format shoudl be using depth value of 24 bits and using StencilBuffer 8Bits (??)
        }, 
        vk::ImageTiling::eOptimal,                         /// We want to use the Optimal Tiling
        vk::FormatFeatureFlagBits::eDepthStencilAttachment); /// Make sure the Format supports the Depth Stencil Attatchment Bit....
    
    /// Create one DepthBuffer per Image in the SwapChain
    for(size_t i = 0; i < swapChainImages.size(); i++)
    {
        /// Create Depth Buffer Image
        this->depthBufferImage[i] = createImage(
            {
                .width = this->swapChainExtent.width, 
                .height = this->swapChainExtent.height, 
                .format = this->depthFormat, 
                .tiling = vk::ImageTiling::eOptimal,                        /// We want to use Optimal Tiling
                .useFlags = vk::ImageUsageFlagBits::eDepthStencilAttachment     /// Image will be used as a Depth Stencil
                            | vk::ImageUsageFlagBits::eInputAttachment,
                .property = vk::MemoryPropertyFlagBits::eDeviceLocal,            /// Image is local to the device, it will not be changed by the HOST (CPU)
                .imageMemory = &this->depthBufferImageMemory[i]
            },
            "depthBufferImage"
            );

        /// Create Depth Buffer Image View
        this->depthBufferImageView[i] = createImageView(this->depthBufferImage[i], depthFormat, vk::ImageAspectFlagBits::eDepth);        
        registerVkObjectDbgInfo("depthBufferImageView["+std::to_string(i)+"]", vk::ObjectType::eImageView, reinterpret_cast<uint64_t>(vk::ImageView::CType(this->depthBufferImageView[i])));
        registerVkObjectDbgInfo("depthBufferImage["+std::to_string(i)+"]", vk::ObjectType::eImage, reinterpret_cast<uint64_t>(vk::Image::CType(this->depthBufferImage[i])));
    }
}

vk::ShaderModule VulkanRenderer::createShaderModule(const std::vector<char> &code) const
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    vk::ShaderModuleCreateInfo shaderCreateInfo = {};
    shaderCreateInfo.setCodeSize(code.size());                                    /// Size of code
    shaderCreateInfo.setPCode(reinterpret_cast<const uint32_t*>(code.data()));    /// pointer of code (of uint32_t pointe rtype //NOLINT:Ok to use Reinterpret cast here

    vk::ShaderModule shaderModule = this->mainDevice.logicalDevice.createShaderModule(shaderCreateInfo);
    return shaderModule;
}

int VulkanRenderer::createTextureImage(const std::string &filename)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    /// Load the image file
    int width = 0;
    int height = 0;
    vk::DeviceSize imageSize = 0;
    stbi_uc* imageData = this->loadTextuerFile(filename, &width,&height, &imageSize);

    ///Create Staging buffer to hold loaded data, ready to copy to device
    vk::Buffer imageStagingBuffer = nullptr;
    //vk::DeviceMemory imageStagingBufferMemory = nullptr;
    VmaAllocation imageStagingBufferMemory = nullptr;
    VmaAllocationInfo allocInfo;

    vengine_helper::createBuffer(
        {
            .physicalDevice = this->mainDevice.physicalDevice, 
            .device = this->mainDevice.logicalDevice, 
            .bufferSize = imageSize, 
            .bufferUsageFlags = vk::BufferUsageFlagBits::eTransferSrc, 
            // .bufferProperties = vk::MemoryPropertyFlagBits::eHostVisible         /// Staging buffer needs to be visible from HOST  (CPU), in order for modification
            //                     |   vk::MemoryPropertyFlagBits::eHostCoherent,   /// not using cache...
            .bufferProperties = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                                | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .buffer = &imageStagingBuffer, 
            //.bufferMemory = &imageStagingBufferMemory,
            .bufferMemory = &imageStagingBufferMemory,
            .allocationInfo = &allocInfo,
            .vma = &vma
        });

    void* data = nullptr; 
    
    if(vmaMapMemory(this->vma, imageStagingBufferMemory, &data) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate Mesh Staging Texture Image Buffer Using VMA!");
    };

    //memcpy(allocInfo.pMappedData, imageData, imageSize);
    memcpy(data, imageData, imageSize);
    vmaUnmapMemory(this->vma , imageStagingBufferMemory);
    

    /// Free image data allocated through stb_image.h 
    stbi_image_free(imageData);

    /// Create image to hold final texture
    vk::Image texImage = nullptr;
    //vk::DeviceMemory texImageMemory = nullptr;
    VmaAllocation texImageMemory = nullptr;
    texImage = createImage(
        {
            .width = static_cast<uint32_t>(width), 
            .height = static_cast<uint32_t>(height), 
            .format = vk::Format::eR8G8B8A8Unorm,               /// use Alpha channel even if image does not have... 
            .tiling =vk::ImageTiling::eOptimal,                /// Same value as the Depth Buffer uses (Dont know if it has to be)
            .useFlags = vk::ImageUsageFlagBits::eTransferDst         /// Data should be transfered to the GPU, from the staging buffer
                        |   vk::ImageUsageFlagBits::eSampled,         /// This image will be Sampled by a Sampler!                         
            .property = vk::MemoryPropertyFlagBits::eDeviceLocal,    /// Image should only be accesable on the GPU 
            .imageMemory = &texImageMemory
        },
        filename // Describing what image is being created, for debug purposes...
        );

    /// - COPY THE DATA TO THE IMAGE -
    /// Transition image to be in the DST, needed by the Copy Operation (Copy assumes/needs image Layout to be in vk::ImageLayout::eTransferDstOptimal state)
    vengine_helper::transitionImageLayout(this->mainDevice.logicalDevice, this->graphicsQueue, this->graphicsCommandPool, 
        texImage,                               /// Image to transition the layout on
        vk::ImageLayout::eUndefined,              /// Image Layout to transition the image from
        vk::ImageLayout::eTransferDstOptimal);  /// Image Layout to transition the image to

    /// Copy Data to image
    vengine_helper::copyImageBuffer(this->mainDevice.logicalDevice, this->graphicsQueue, this->graphicsCommandPool, imageStagingBuffer, texImage, width, height);

    /// Transition iamge to be shader readable for shader usage
    vengine_helper::transitionImageLayout(this->mainDevice.logicalDevice, this->graphicsQueue, this->graphicsCommandPool, 
        texImage,
        vk::ImageLayout::eTransferDstOptimal,       /// Image layout to transition the image from; this is the same as we transition the image too before we copied buffer!
        vk::ImageLayout::eShaderReadOnlyOptimal);  /// Image Layout to transition the image to; in order for the Fragment Shader to read it!         

    /// Add texture data to vector for reference 
    textureImages.push_back(texImage);
    textureImageMemory.push_back(texImageMemory);

    /// Destroy and Free the staging buffer + staging buffer memroy
    this->mainDevice.logicalDevice.destroyBuffer(imageStagingBuffer);
    //this->mainDevice.logicalDevice.freeMemory(imageStagingBufferMemory);
    vmaFreeMemory(this->vma, imageStagingBufferMemory);

    // Return index of last pushed image!
    return static_cast<int>(textureImages.size()) -1; 
}

int VulkanRenderer::createTexture(const std::string &filename)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    /// Create Texture Image and get its Location in array
    int textureImageLoc = createTextureImage(filename);

    /// Create Image View
    vk::ImageView imageView = createImageView(
        this->textureImages[textureImageLoc],   /// The location of the Image in our textureImages vector
        vk::Format::eR8G8B8A8Unorm,               /// Format for rgba 
        vk::ImageAspectFlagBits::eColor);             /// Image is used for color presentation (??)

    /// Add the Image View to our vector with Image views
    this->textureImageViews.push_back(imageView);

    /// Create Texture Descriptor
    int descriptorLoc = createTextureDescriptor(imageView);

    /// Return index of Texture Descriptor that was just created
    return descriptorLoc;
}

int VulkanRenderer::createTextureDescriptor(vk::ImageView textureImage)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    /// 
    vk::DescriptorSet descriptorSet = nullptr;

    /// Descriptor Set Allocation Info
    vk::DescriptorSetAllocateInfo setAllocateInfo;
    setAllocateInfo.setDescriptorPool(this->samplerDescriptorPool);
    setAllocateInfo.setDescriptorSetCount(uint32_t (1));
    setAllocateInfo.setPSetLayouts(&this->samplerDescriptorSetLayout);

    /// Allocate Descriptor Sets
    descriptorSet = this->mainDevice.logicalDevice.allocateDescriptorSets(setAllocateInfo)[0];

    /// Tedxture Image info
    vk::DescriptorImageInfo imageInfo;
    imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);     /// The Image Layout when it is in use
    imageInfo.setImageView(textureImage);                                 /// Image to be bind to set
    imageInfo.setSampler(this->textureSampler);                         /// the Sampler to use for this Descriptor Set

    /// Descriptor Write Info
    vk::WriteDescriptorSet descriptorWrite;
    descriptorWrite.setDstSet(descriptorSet);
    descriptorWrite.setDstArrayElement(uint32_t (0));
    descriptorWrite.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
    descriptorWrite.setDescriptorCount(uint32_t (1));
    descriptorWrite.setPImageInfo(&imageInfo);

    /// Update the new Descriptor Set
    this->mainDevice.logicalDevice.updateDescriptorSets(
        uint32_t(1),
        &descriptorWrite,
        uint32_t(0),
        nullptr
    );

    /// Add descriptor Set to our list of descriptor Sets
    samplerDescriptorSets.push_back(descriptorSet);

    ///Return the last created Descriptor set
    return static_cast<int>(samplerDescriptorSets.size() -1); 
}

int VulkanRenderer::createModel(const std::string& modelFile)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif    
    /// Import Model Scene
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(
        (DEF<std::string>(P_MODELS)+modelFile).c_str(),
        aiProcess_Triangulate               /// Ensures that ALL objects will be represented as Triangles
        | aiProcess_FlipUVs                 /// Flips the texture UV values, to be same as how we use them
        | aiProcess_JoinIdenticalVertices   /// Saves memory by making sure no dublicate vertices exists
        );

    if(scene == nullptr){throw std::runtime_error("Failed to load model ("+modelFile+")");}

    /// Get vector of all materials 
    std::vector<std::string> textureNames = Model::loadMaterials(scene);

    /// Handle empty texture 
    std::vector<int> matToTexture(textureNames.size());

    for(size_t i = 0; i < textureNames.size(); i++){
        
        if(textureNames[i].empty())
        {
            matToTexture[i] = 0; // Use default textures for models if textures are missing
        }
        else
        {
            /// Create texture, use the index returned by our createTexture function
            matToTexture[i] = createTexture(textureNames[i]);
        }
    }

    /// Load in all meshes
    std::vector<Mesh> modelMeshes = Model::getMeshesFromNodeTree(&this->vma,this->mainDevice.physicalDevice, this->mainDevice.logicalDevice, this->graphicsQueue, this->graphicsCommandPool, scene, matToTexture);

    /// Create Model, add to list
    Model model = Model(&this->vma,this->mainDevice.physicalDevice, this->mainDevice.logicalDevice, this->graphicsQueue, this->graphicsCommandPool,modelMeshes);
    modelList.emplace_back(model);

    return static_cast<int>(modelList.size())-1;

}

stbi_uc* VulkanRenderer::loadTextuerFile(const std::string &filename, int* width, int* height, vk::DeviceSize* imageSize)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    /// Number of Channels the image uses, will not be used but could be used in the future
    int channels = 0;
    using namespace vengine_helper::config;
    /// Load pixel Data from file to image
    std::string fileLoc  = DEF<std::string>(P_TEXTURES) + filename;
    stbi_uc* image = stbi_load(
            fileLoc.c_str(),
            width,
            height,
            &channels,          /// In case we want to  use channels, its stored in channels
            STBI_rgb_alpha );   /// force image to be in format : RGBA

    if(image == nullptr){throw std::runtime_error("Failed to load a Texture file! ("+filename+")");}
    

    /// Calculate image sisze using given and known data
    *imageSize = static_cast<uint32_t>((*width) * (*height) * 4); /// width times height gives us size per channel, we have 4 channels! (rgba)

    return image;
    
}

void VulkanRenderer::rendererGameLoop()
{
    SDL_Event event;  
    bool quitting = false;
    while (!quitting) {
        this->eventBuffer.clear();
        while (SDL_PollEvent(&event) != 0) {    
            ImGui_ImplSDL2_ProcessEvent(&event);        
            if (event.type == SDL_QUIT ||  
            (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(this->window.sdl_window)))
            {
                quitting = true;
            }
            if(event.type == SDL_KEYDOWN){
                switch(event.key.keysym.sym){
                    case SDLK_HOME:
                        std::cout << "Home was pressed! generating vma dump" << "\n";
                        this->generateVmaDump();
                        
                    default: ;
                }
            }
            this->eventBuffer.emplace_back(event);
        }
        //SDL_PollEvent(&event);
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL2_NewFrame(this->window.sdl_window);
        ImGui::NewFrame();
        
        this->gameLoopFunction(this->eventBuffer);

        this->draw(); 
#ifndef VENGINE_NO_PROFILING
        FrameMark;
#endif
    }
    
}

VulkanRenderer::VulkanRenderer()
{
    loadConfIntoMemory();
}

void VulkanRenderer::createRenderPass_Base() 
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    /// Array of our subPasses    
    std::array<vk::SubpassDescription2, 2> subPasses {};     

    /// - ATTATCHMENTS - 
    /// SUBPASS 1 ATTACHMENTS (Input attachments) and ATTACHMENT REFERENCES

    /// Color Attachment (Input)
    vk::AttachmentDescription2 colorAttachment {};
    colorAttachment.setFormat(this->chooseSupportedFormat(
        {
            vk::Format::eR8G8B8A8Unorm
        }, 
        vk::ImageTiling::eOptimal,         
        vk::FormatFeatureFlagBits::eColorAttachment));        
        
    colorAttachment.setSamples(vk::SampleCountFlagBits::e1);
    colorAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);      /// When we start the renderpass, first thing to do is to clear since there is no values in it yet
    colorAttachment.setStoreOp(vk::AttachmentStoreOp::eStore); /// How to store it after the RenderPass; We dont care! But we do care what happens after the first SubPass! (not handled here)
    colorAttachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
    colorAttachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
    colorAttachment.setInitialLayout(vk::ImageLayout::eUndefined);        /// We dont care what the image layout is when we start. But we do care about what layout it is when it enter the first SubPass! (not handled here)
    //colorAttachment.setFinalLayout(vk::ImageLayout::eAttachmentOptimal); /// Should be the same value as it was after the subpass finishes (??)
    colorAttachment.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal); ///(!!) Should be the same value as it was after the subpass finishes (??)

    

    /// Depth Attatchment Of Render Pass 
    vk::AttachmentDescription2 depthAttatchment{};
    depthAttatchment.setFormat(this->depthFormat);
    depthAttatchment.setSamples(vk::SampleCountFlagBits::e1);
    depthAttatchment.setLoadOp(vk::AttachmentLoadOp::eClear);      /// Clear Buffer Whenever we try to load data into (i.e. clear before use it!)
    depthAttatchment.setStoreOp(vk::AttachmentStoreOp::eDontCare); /// Whenever it's used, we dont care what happens with the data... (we dont present it or anything)
    depthAttatchment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);  /// Even though the Stencil i present, we dont plan to use it. so we dont care    
    depthAttatchment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);     /// Even though the Stencil i present, we dont plan to use it. so we dont care
    depthAttatchment.setInitialLayout(vk::ImageLayout::eUndefined);        /// We don't care how the image layout is initially, so let it be undefined
    depthAttatchment.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal); /// Final layout should be Optimal for Depth Stencil attachment!

    /// Color Attachment (input) Reference
    vk::AttachmentReference2 colorAttachmentReference {};    
    colorAttachmentReference.setAttachment(uint32_t(1));            /// Match the number/ID of the Attachment to the index of the FrameBuffer!
    colorAttachmentReference.setLayout(vk::ImageLayout::eColorAttachmentOptimal); /// The Layout the Subpass must be in! 

    /// Depth Attachment (input) Reference
    vk::AttachmentReference2 depthAttachmentReference {};
    depthAttachmentReference.setAttachment(uint32_t(2)); 
    depthAttachmentReference.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal); /// The layout the subpass must be in! Should be same as 'final layout'(??)

    /// Setup Subpass 1
    subPasses[0].setPipelineBindPoint(vk::PipelineBindPoint::eGraphics); 
    subPasses[0].setColorAttachmentCount(uint32_t(1)); 
    subPasses[0].setPColorAttachments(&colorAttachmentReference); 
    subPasses[0].setPDepthStencilAttachment(&depthAttachmentReference); 

    /// SUBPASS 2 ATTACHMENTS and ATTACHMENT REFERENCES

    /// Color Attachment SwapChain
    vk::AttachmentDescription2 swapchainColorAttachment{};
    swapchainColorAttachment.setFormat(swapChainImageFormat);              /// Format to use for attachgment
    swapchainColorAttachment.setSamples(vk::SampleCountFlagBits::e1);            /// Number of samples to write for multisampling
    swapchainColorAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);       /// Descripbes what to do with attachment before rendeing
    swapchainColorAttachment.setStoreOp(vk::AttachmentStoreOp::eStore);     /// Describes what to do with Attachment after rendering
    swapchainColorAttachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);     /// Describes what to do with stencil before rendering
    swapchainColorAttachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);   /// Describes what to do with stencil affter rendering

    /// Framebuffer data will be stored as an image, but images can be given different data layouts
    /// to give optimal usefor certain operations
    swapchainColorAttachment.setInitialLayout(vk::ImageLayout::eUndefined);      /// image data layout before render pass starts
    //swapchainColorAttachment.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);  /// Image data layout after render pass (to change to)
    swapchainColorAttachment.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);  /// edit: After adding Imgui, this renderpass will no longer be the presenting one, thus we change final layout! 

    vk::AttachmentReference2 swapchainColorAttachmentReference = {};
    swapchainColorAttachmentReference.setAttachment(uint32_t(0));                                       /// Attatchment on index 0 of array
    swapchainColorAttachmentReference.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    /// Create two more Attachment References; These describes what the subpass will take input from
    std::array<vk::AttachmentReference2,2 > inputReferences{};
    inputReferences[0].setAttachment(uint32_t(1));                                        /// The Attachment index (Color Input)
    inputReferences[0].setLayout(vk::ImageLayout::eShaderReadOnlyOptimal); /// reuse the attachment (1), make sure it's uses SHADER_READ_ONLY_OPTIMAL layout!
    inputReferences[0].setAspectMask(vk::ImageAspectFlagBits::eColor);//(!!)
    inputReferences[0].setPNext(nullptr); //(!!)(??)
    inputReferences[1].setAttachment(uint32_t(2));                                        /// The Attachment index (Depth Input)
    inputReferences[1].setAspectMask(vk::ImageAspectFlagBits::eDepth);//(!!)    
    inputReferences[1].setLayout(vk::ImageLayout::eReadOnlyOptimal); /// reuse the attachment (2), make sure it's uses SHADER_READ_ONLY_OPTIMAL layout!
    inputReferences[1].setPNext(nullptr); //(!!)(??)

    /// Setup Subpass 2
    subPasses[1].setPipelineBindPoint(vk::PipelineBindPoint::eGraphics); 
    subPasses[1].setColorAttachmentCount(uint32_t(1)); 
    subPasses[1].setPColorAttachments(&swapchainColorAttachmentReference); 
    subPasses[1].setInputAttachmentCount(static_cast<uint32_t>(inputReferences.size()));    // How many Inputs Subpass 2 will have
    subPasses[1].setPInputAttachments(inputReferences.data());                           // The Input Attachments References Subpass 2 will use
    

/*  ///! Old version, when we only used One subpass! 
    /// Color attatchments of render pass
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = swapChainImageFormat;              /// Format to use for attachgment
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;            /// Number of samples to write for multisampling
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;       /// Descripbes what to do with attachment before rendeing
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;     /// Describes what to do with Attachment after rendering
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;     /// Describes what to do with stencil before rendering
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;   /// Describes what to do with stencil affter rendering

    /// Framebuffer data will be stored as an image, but images can be given different data layouts
    /// to give optimal usefor certain operations
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;      /// image data layout before render pass starts
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;  /// Image data layout after render pass (to change to)

    /// Depth Attatchment Of Render Pass 
    VkAttachmentDescription depthAttatchment{};
    depthAttatchment.format         = this->depthFormat;
    depthAttatchment.samples        = VK_SAMPLE_COUNT_1_BIT;
    depthAttatchment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;      /// Clear Buffer Whenever we try to load data into (i.e. clear before use it!)
    depthAttatchment.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE; /// Whenever it's used, we dont care what happens with the data... (we dont present it or anything)
    depthAttatchment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;  /// Even though the Stencil i present, we dont plan to use it. so we dont care
    depthAttatchment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;     /// Even though the Stencil i present, we dont plan to use it. so we dont care
    depthAttatchment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;        /// We don't care how the image layout is initially, so let it be undefined
    depthAttatchment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; /// Final layout should be Optimal for Depth Stencil attachment!
    
    /// - REFERENCES - 
    /// attachment reference uses an attachment index that refers to iundex in the attachment list passed to renderPassCreateINfo
    VkAttachmentReference colorAttachmentReference = {};
    colorAttachmentReference.attachment = 0;                                       /// Attatchment on index 0 of array
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    /// Depth Attatchemnt Reference 
    VkAttachmentReference depthAttatchmentReference{};
    depthAttatchmentReference.attachment = 1;                                       /// Attatchment on index 1 of array
    depthAttatchmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; /// Note this does not change between the Attachment Reference and the Attachment!

    /// Information about a particular subpass the render pass is using
    VkSubpassDescription subPass = {};
    subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subPass.colorAttachmentCount = 1;
    subPass.pColorAttachments = &colourAttachmentReference;    
    subPass.pDepthStencilAttachment = &depthAttatchmentReference;       /// Set the One And Only Depth Stencil attatchment to be used by our subpass!
    
    /// Need to determine when layout transition occour using subpass dependencies
    std::array<VkSubpassDependency, 2> subpassDependencies;

    /// Conversion from VK_IMAGE_LAYOUT_UNDEFINED to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
     /// Transition must happen after...
    subpassDependencies[0].srcSubpass   = VK_SUBPASS_EXTERNAL;                    /// Subpass index (VK_SUBPASS_EXTERNAL = Special valye meaning outside of renderpass)=
    subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT; /// Pipeline Stage
    subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;           /// Stage Access Mask (Memory access)
    /// But must happen before
    subpassDependencies[0].dstSubpass = 0;
    subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT  | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpassDependencies[0].dependencyFlags = 0;

    /// Conversion from VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL to VK_IMAGE_LAYOUT_UNDEFINED
    /// Transition must happen after...
    subpassDependencies[1].srcSubpass = 0; //SourceSubpass is not Subpass On index 0
    subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT  | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    /// But must happen before
    subpassDependencies[1].dstSubpass    =  VK_SUBPASS_EXTERNAL;
    subpassDependencies[1].dstStageMask  =  VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    subpassDependencies[1].dependencyFlags = 0;
*/

/*
    ///TODO: If subpass will be used, then the Write After Write warnings should be fixed...
    /// Need to determine when layout transition occour using subpass dependencies
    std::array<vk::SubpassDependency2, 3> subpassDependencies{};
    /// Conversion from VK_IMAGE_LAYOUT_UNDEFINED to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    /// Transition must happen after...
    subpassDependencies[0].setSrcSubpass(VK_SUBPASS_EXTERNAL);                /// Subpass index (VK_SUBPASS_EXTERNAL = Special valye meaning outside of renderpass)        
    subpassDependencies[0].setSrcStageMask(vk::PipelineStageFlagBits::eBottomOfPipe); /// Pipeline Stage
    subpassDependencies[0].setSrcAccessMask(vk::AccessFlagBits::eMemoryRead);            /// Stage Access Mask (Memory access)
    /// But must happen before
    subpassDependencies[0].setDstSubpass(uint32_t(0));
    subpassDependencies[0].setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    subpassDependencies[0].setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);
    subpassDependencies[0].setDependencyFlags(vk::DependencyFlagBits::eByRegion); 
    
    /// Subpass 1 layout (color/depth) to Subpass 2 layout (shader read)
    /// Transition must happen after...
    subpassDependencies[1].setSrcSubpass(uint32_t(0));                                            /// After Subpass 1 (subpass 1 is on index 0...)
    subpassDependencies[1].setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);/// After Attachment is ready for Shader (i.e. value of dstStageMask for previous Dependency)    
    subpassDependencies[1].setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);         /// After Attachment has been written to
    //// But must happen before
    subpassDependencies[1].setDstSubpass(uint32_t(1));                                            /// Before Subpass 2 (subpass 2 is on index 1...)
    subpassDependencies[1].setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader);        /// Before we reach the Fragment Shader 
    subpassDependencies[1].setDstAccessMask(vk::AccessFlagBits::eShaderRead);                    /// Before the Shader has read from it  
    subpassDependencies[1].setDependencyFlags(vk::DependencyFlagBits::eByRegion); 


    /// Conversion from VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL to VK_IMAGE_LAYOUT_UNDEFINED
    /// Transition must happen after...
    subpassDependencies[2].setSrcSubpass(uint32_t(1)); // that was wrong ... should be 1 it seems according to a comment-> SourceSubpass is not Subpass On index 0
    subpassDependencies[2].setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput 
                                            |vk::PipelineStageFlagBits::eLateFragmentTests );
    subpassDependencies[2].setSrcAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite);
    /// But must happen before
    subpassDependencies[2].setDstSubpass(VK_SUBPASS_EXTERNAL);
    subpassDependencies[2].setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader);
    subpassDependencies[2].setDstAccessMask(vk::AccessFlagBits::eInputAttachmentRead);
    subpassDependencies[2].setDependencyFlags(vk::DependencyFlagBits::eByRegion); 
*/

 /// Replacing all subpass dependencies with this results in no errors/warnings. But I dont know if it is correct... Seems to be the same if subpassDependency[0] and [2] are removed from the code above...
    std::array<vk::SubpassDependency2, 1> subpassDependencies{};
    subpassDependencies[0].setSrcSubpass(0);
    subpassDependencies[0].setDstSubpass(1);
    subpassDependencies[0].setSrcStageMask(vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests);
    subpassDependencies[0].setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader);
    subpassDependencies[0].setSrcAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite);
    subpassDependencies[0].setDstAccessMask(vk::AccessFlagBits::eInputAttachmentRead);
    subpassDependencies[0].setDependencyFlags(vk::DependencyFlagBits::eByRegion);

    /// Vector with the Attatchments
    std::array<vk::AttachmentDescription2,3> attatchments{
        swapchainColorAttachment,   /// Attachment on index 0 of array : SwapChain Color
        colorAttachment,            /// Attachment on index 1 of array : Color (input to SubPass 2)
        depthAttatchment            /// Attachment on index 2 of array : Depth (input to SubPass 2)
    };

    ///Create info for render pass
    vk::RenderPassCreateInfo2 renderPassCreateInfo;
    renderPassCreateInfo.setAttachmentCount(static_cast<uint32_t>(attatchments.size()));
    renderPassCreateInfo.setPAttachments(attatchments.data());
    renderPassCreateInfo.setSubpassCount(static_cast<uint32_t>(subPasses.size()));
    renderPassCreateInfo.setPSubpasses(subPasses.data());
    renderPassCreateInfo.setDependencyCount(static_cast<uint32_t> (subpassDependencies.size()));
    renderPassCreateInfo.setPDependencies(subpassDependencies.data());

    this->renderPass_base = this->mainDevice.logicalDevice.createRenderPass2(renderPassCreateInfo);

    registerVkObjectDbgInfo("The RenderPass", vk::ObjectType::eRenderPass, reinterpret_cast<uint64_t>(vk::RenderPass::CType(this->renderPass_base)));
    
}

void VulkanRenderer::createRenderPass_Imgui()
{
    vk::AttachmentDescription2 attachment{};
    attachment.setFormat(this->swapChainImageFormat);
    attachment.setSamples(vk::SampleCountFlagBits::e1);
    attachment.setLoadOp(vk::AttachmentLoadOp::eDontCare);
    attachment.setStoreOp(vk::AttachmentStoreOp::eStore);
    attachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
    attachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
    attachment.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal);
    //attachment.setInitialLayout(vk::ImageLayout::eUndefined);
    
    attachment.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

    vk::AttachmentReference2 attachment_reference{};
    attachment_reference.setAttachment(uint32_t(0));
    attachment_reference.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    vk::SubpassDescription2 subpass{};
    subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
    subpass.setColorAttachmentCount(uint32_t(1));
    subpass.setPColorAttachments(&attachment_reference);

    vk::SubpassDependency2 subpassDependecy{};
    subpassDependecy.setSrcSubpass(VK_SUBPASS_EXTERNAL);
    subpassDependecy.setDstSubpass(uint32_t(0));
    subpassDependecy.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput); // Wait until all other graphics have been rendered
    subpassDependecy.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    subpassDependecy.setSrcAccessMask(vk::AccessFlags());
    //subpassDependecy.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite); //TODO: check this...
    subpassDependecy.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

    vk::RenderPassCreateInfo2 renderpassCreateinfo{};
    renderpassCreateinfo.setAttachmentCount(uint32_t (1));
    renderpassCreateinfo.setPAttachments(&attachment);
    renderpassCreateinfo.setSubpassCount(uint32_t (1));
    renderpassCreateinfo.setPSubpasses(&subpass);
    renderpassCreateinfo.setDependencyCount(uint32_t(1));
    renderpassCreateinfo.setPDependencies(&subpassDependecy);
    this->renderPass_imgui = this->mainDevice.logicalDevice.createRenderPass2(renderpassCreateinfo);
}

void VulkanRenderer::createDescriptorSetLayout()
{    
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    /// - CREATE UNIFORM VALUES DESCRIPTOR SET LAYOUT -

    /// UboViewProjection binding Info
    vk::DescriptorSetLayoutBinding vpLayoutBinding;
    vpLayoutBinding.setBinding(uint32_t (0));                                           /// Describes which Binding Point in the shaders this layout is being bound to
    vpLayoutBinding.setDescriptorType(vk::DescriptorType::eUniformBuffer);    /// Type of descriptor (Uniform, Dynamic uniform, image Sampler, etc.)
    vpLayoutBinding.setDescriptorCount(uint32_t(1));                                   /// Amount of actual descriptors we're binding, where just binding one; our MVP struct
    vpLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eVertex);               /// What Shader Stage we want to bind our Descriptor set to
    vpLayoutBinding.setPImmutableSamplers(nullptr);//vknullhandle??          /// Used by Textures; whether or not the Sampler should be Immutable

/*  /// Left for Reference; we dont use Dynamic Uniform Buffers for our Model Matrix anymore.
    /// Model binding info
    VkDescriptorSetLayoutBinding modelLayoutBinding{};
    modelLayoutBinding.binding = 1;                                                 /// This is part of Descriptor Set 0, thus we need to use another binding 
    modelLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;  /// The Model Unifom Buffer is Dynamic, since it will change every drawcall
    modelLayoutBinding.descriptorCount = 1;
    modelLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    modelLayoutBinding.pImmutableSamplers = nullptr;
*/

    /// Adding the Bindings to a Vector in order to submit all the DescriptorSetLayout Bindings! 
    std::vector<vk::DescriptorSetLayoutBinding> layoutBindings {
        vpLayoutBinding
        /// Left for Reference; we dont use Dynamic Uniform Buffers for our Model Matrix anymore.
        ///,modelLayoutBinding
    };

    /// Create Descriptor Set Layout with given bindings
    vk::DescriptorSetLayoutCreateInfo layoutCreateInfo{};
    layoutCreateInfo.setBindingCount(static_cast<uint32_t>(layoutBindings.size()));  /// Number of Binding infos
    layoutCreateInfo.setPBindings(layoutBindings.data());                            /// Array containing the binding infos

    /// Create Descriptor Set Layout
    this->descriptorSetLayout = this->mainDevice.logicalDevice.createDescriptorSetLayout(layoutCreateInfo);
    
    registerVkObjectDbgInfo("DescriptorSetLayout ViewProjection", vk::ObjectType::eDescriptorSetLayout, reinterpret_cast<uint64_t>(vk::DescriptorSetLayout::CType(this->descriptorSetLayout)));

    /// - CREATE TEXTURE SAMPLER DESCRIPTOR SET LAYOUT -
    /// Texture Binding Info
    vk::DescriptorSetLayoutBinding samplerLayoutBinding;
    samplerLayoutBinding.setBinding(uint32_t (0));                                   /// This can be 0 too, as it will be for a different Descriptor Set, Descriptor set 1 (previous was Descriptor Set 0)! 
    samplerLayoutBinding.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
    samplerLayoutBinding.setDescriptorCount(uint32_t (1));               
    samplerLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eFragment);     /// The Stage the descriptor layout will pass to will be the Fragment Shader
    samplerLayoutBinding.setPImmutableSamplers(nullptr);

    /// Create a Descriptor Set Layout with given bindings for texture
    vk::DescriptorSetLayoutCreateInfo textureLayoutCreateInfo;
    textureLayoutCreateInfo.setBindingCount(uint32_t (1));
    textureLayoutCreateInfo.setPBindings(&samplerLayoutBinding);

    /// create Descriptor Set Layout
    this->samplerDescriptorSetLayout = this->mainDevice.logicalDevice.createDescriptorSetLayout(textureLayoutCreateInfo);
    registerVkObjectDbgInfo("DescriptorSetLayout SamplerTexture", vk::ObjectType::eDescriptorSetLayout, reinterpret_cast<uint64_t>(vk::DescriptorSetLayout::CType(this->samplerDescriptorSetLayout)));

    /// CREATE INPUT ATTACHMENT IMAGE DESCRIPTOR SET LAYOUT
    /// Colour Input Binding
    vk::DescriptorSetLayoutBinding colorInputLayoutBinding;
    colorInputLayoutBinding.setBinding(uint32_t (0));                                            /// Binding 0 for Set 0, but for a new pipeline
    colorInputLayoutBinding.setDescriptorCount(uint32_t (1));
    colorInputLayoutBinding.setDescriptorType(vk::DescriptorType::eInputAttachment);   /// Describes that the Descriptor is used by Input Attachment
    colorInputLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eFragment);              /// Passed into the Fragment Shader Stage

    vk::DescriptorSetLayoutBinding depthInputLayoutBinding;
    depthInputLayoutBinding.setBinding(uint32_t (1));
    depthInputLayoutBinding.setDescriptorCount(uint32_t (1)); 
    depthInputLayoutBinding.setDescriptorType(vk::DescriptorType::eInputAttachment);
    depthInputLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eFragment);

    std::array<vk::DescriptorSetLayoutBinding, 2> inputLayoutBindings {   /// Bindings for the Second Subpass input attachments (??)
        colorInputLayoutBinding,
        depthInputLayoutBinding
    };

    /// Create a Descriptor Set Layout for input attachments
    vk::DescriptorSetLayoutCreateInfo inputCreateInfo;
    inputCreateInfo.setBindingCount(static_cast<uint32_t>(inputLayoutBindings.size()));
    inputCreateInfo.setPBindings(inputLayoutBindings.data());
    inputCreateInfo.setFlags(vk::DescriptorSetLayoutCreateFlags());    
    inputCreateInfo.pNext = nullptr;

    /// create Descriptor Set Layout
    this->inputSetLayout = this->mainDevice.logicalDevice.createDescriptorSetLayout(inputCreateInfo);
    registerVkObjectDbgInfo("DescriptorSetLayout SubPass2_input", vk::ObjectType::eDescriptorSetLayout, reinterpret_cast<uint64_t>(vk::DescriptorSetLayout::CType(this->inputSetLayout)));
}

void VulkanRenderer::createPushConstantRange()
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    /// Define the Push Constants values
    this->pushConstantRange.setStageFlags(vk::ShaderStageFlagBits::eVertex);    /// Push Constant should be available in the Vertex Shader!
    this->pushConstantRange.setOffset(uint32_t (0));                             /// Offset into the given data that our Push Constant value is (??)
    this->pushConstantRange.setSize(sizeof(ModelMatrix));                 /// Size of the Data being passed
}

void VulkanRenderer::createFrameBuffers() 
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    /// Resize framebuffer count to equal swap chain image count
    swapChainFrameBuffers.resize(swapChainImages.size());

    /// Create a framebuffer for each swap chain image
    for (size_t i = 0; i < swapChainFrameBuffers.size();i++) {

        std::array<vk::ImageView, 3> attachments = {
                swapChainImages[i].imageView,   /// Attatchment on index 0 of array : swapchain image
                this->colorBufferImageView[i],   /// Attachement on index 1 of array : color
                this->depthBufferImageView[i]  /// Attatchment on index 2 of array : depth
        };

        vk::FramebufferCreateInfo framebufferCreateInfo;
        framebufferCreateInfo.setRenderPass(renderPass_base);                                      /// Render pass layout the framebuyfffeer will be used with
        framebufferCreateInfo.setAttachmentCount(static_cast<uint32_t>(attachments.size()));
        framebufferCreateInfo.setPAttachments(attachments.data());                            /// List of attatchemnts (1:1 with render pass)
        framebufferCreateInfo.setWidth(swapChainExtent.width);
        framebufferCreateInfo.setHeight( swapChainExtent.height);
        framebufferCreateInfo.setLayers(uint32_t (1));

        this->swapChainFrameBuffers[i] = mainDevice.logicalDevice.createFramebuffer(framebufferCreateInfo);        
        registerVkObjectDbgInfo("SwapchainFramebuffer["+std::to_string(i)+"]", vk::ObjectType::eFramebuffer, reinterpret_cast<uint64_t>(vk::Framebuffer::CType(this->swapChainFrameBuffers[i])));

    }
}

void VulkanRenderer::createCommandPool()
 {
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    /// Get indices of QueueFamilyDevice
    QueueFamilyIndices queueFamilyIndices = this->getQueueFamilies(this->mainDevice.physicalDevice);

    vk::CommandPoolCreateInfo poolInfo = {};
    poolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);   /// Enables us to reset our CommandBuffers 
                                                                        //// if they were allocated from this CommandPool!
                                                                        //// To make use of this feature you also have to activate in during Recording! (??)
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;      /// Queue family type that buffers from this command pool wil luse

    /// Create a graphics Queue Family Command Pool
    this->graphicsCommandPool = this->mainDevice.logicalDevice.createCommandPool(poolInfo);
    registerVkObjectDbgInfo("CommandPool Presentation/Graphics", vk::ObjectType::eCommandPool, reinterpret_cast<uint64_t>(vk::CommandPool::CType(this->graphicsCommandPool)));

}

void VulkanRenderer::createCommandBuffers() 
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    /// Resize command buffer count to have one for each framebuffer
    commandBuffers.resize(swapChainFrameBuffers.size());

    vk::CommandBufferAllocateInfo cbAllocInfo;
    cbAllocInfo.setCommandPool(graphicsCommandPool);
    cbAllocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
    /*! PRIMARY : means that the commandBuffer will be given to a queue and run of the queue, it will only be called by that Queue...
     * SECONDARY: Means that the commandBuffer cant be passed to a Queue, it can only be called by another commandBuffer... (Commadnbuffer within a commandBuffer...)
     *              A secondary CommandBuffer can be executed by another Primary commandBuffer with 'vkCmdExecuteCommands(buffer)'
     * tldr:
     *  Primary   = executed by Queues
     *  Secondary = Executed by another Primary CommandBuffer
     */
    cbAllocInfo.setCommandBufferCount(static_cast<uint32_t>(commandBuffers.size()));

    /// Allocate command Buffers and place handles in array of buffers
    this->commandBuffers = this->mainDevice.logicalDevice.allocateCommandBuffers(cbAllocInfo);

    for(size_t i = 0; i < swapChainFrameBuffers.size(); i++)
    {
        registerVkObjectDbgInfo("Graphics CommandBuffer["+std::to_string(i)+"]", vk::ObjectType::eCommandBuffer, reinterpret_cast<uint64_t>(vk::CommandBuffer::CType(this->commandBuffers[i])));
    }

    /*!NOTE:
     * For this 'create function' we actually do not create anything, we just allocate!
     * Thus we do not need to destroy anything in the cleanup function,
     * This will be done automatically when we destroy our commandPool!
     * */

}

void VulkanRenderer::createSynchronisation()
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    /// Create Semaphores for each Transition a Image can be in
    imageAvailable.resize(MAX_FRAME_DRAWS);
    renderFinished.resize(MAX_FRAME_DRAWS);
    drawFences.resize(MAX_FRAME_DRAWS);

    ///  Semaphore Creation information
    vk::SemaphoreCreateInfo semaphoreCreateInfo; 
    

    /// Fence Creation Information
    vk::FenceCreateInfo fenceCreateInfo;
    fenceCreateInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);           /// Make sure the Fence is initially open!

    for(int i = 0; i < MAX_FRAME_DRAWS; i++){

        this->imageAvailable[i] = mainDevice.logicalDevice.createSemaphore(semaphoreCreateInfo);
        registerVkObjectDbgInfo("Semaphore imageAvailable["+std::to_string(i)+"]", vk::ObjectType::eSemaphore, reinterpret_cast<uint64_t>(vk::Semaphore::CType(this->imageAvailable[i])));
        
        this->renderFinished[i] = mainDevice.logicalDevice.createSemaphore(semaphoreCreateInfo);
        registerVkObjectDbgInfo("Semaphore renderFinished["+std::to_string(i)+"]", vk::ObjectType::eSemaphore, reinterpret_cast<uint64_t>(vk::Semaphore::CType(this->renderFinished[i])));
            
        this->drawFences[i] = mainDevice.logicalDevice.createFence(fenceCreateInfo);
        registerVkObjectDbgInfo("Fence drawFences["+std::to_string(i)+"]", vk::ObjectType::eFence, reinterpret_cast<uint64_t>(vk::Fence::CType(this->drawFences[i])));
    }
}

void VulkanRenderer::createTextureSampler()
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    /// Sampler Creation info;
    vk::SamplerCreateInfo samplerCreateInfo;
    samplerCreateInfo.setMagFilter(vk::Filter::eLinear);                        /// How the sampler will sample from a texture when it's getting closer
    samplerCreateInfo.setMinFilter(vk::Filter::eLinear);                        /// How the sampler will sample from a texture when it's getting further away
    samplerCreateInfo.setAddressModeU(vk::SamplerAddressMode::eRepeat);        /// How the texture will be Wrapped in U (x) direction
    samplerCreateInfo.setAddressModeV(vk::SamplerAddressMode::eRepeat);        /// How the texture will be Wrapped in V (y) direction
    samplerCreateInfo.setAddressModeW(vk::SamplerAddressMode::eRepeat);        /// How the texture will be Wrapped in W (z) direction
    samplerCreateInfo.setBorderColor(vk::BorderColor::eIntOpaqueBlack);      /// Color of what is around the texture (in case of Repeat, it wont be used)
    samplerCreateInfo.setUnnormalizedCoordinates(VK_FALSE);                   /// We want to used Normalised Coordinates (between 0 and 1), so unnormalized coordinates must be false... 
    samplerCreateInfo.setMipmapMode(vk::SamplerMipmapMode::eLinear);           /// How the mipmap mode will switch between the mipmap images (interpolate between images), (we dont use it, but we set it up)
    samplerCreateInfo.setMipLodBias(0.F);                                     /// Level of detail bias for mip level...
    samplerCreateInfo.setMinLod(0.F);                                     /// Minimum level of Detail to pick mip level
    samplerCreateInfo.setMaxLod(0.F);                                     /// Maxiumum level of Detail to pick mip level
    samplerCreateInfo.setAnisotropyEnable(VK_TRUE);                           /// Enable Anisotropy; take into account the angle of a surface is being viewed from and decide details based on that (??)
    //samplerCreateInfo.setAnisotropyEnable(VK_FALSE);                           /// Disable Anisotropy; Cause Performance Issues according to validation... 
                                                                            /// TODO: Check how anisotrophy can be used without causing validation errors... ? 
    samplerCreateInfo.setMaxAnisotropy(DEF<float>(SAMPL_MAX_ANISOSTROPY)); /// Level of Anisotropy; 16 is a common option in the settings for alot of Games 

    this->textureSampler = this->mainDevice.logicalDevice.createSampler(samplerCreateInfo);
    registerVkObjectDbgInfo("Texture Sampler", vk::ObjectType::eSampler, reinterpret_cast<uint64_t>(vk::Sampler::CType(this->textureSampler)));
}

void VulkanRenderer::createUniformBuffers()
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    /// viewProjection_UniformBuffer size will be size of the view and Projection Members (will offset to access)
    vk::DeviceSize viewProjection_buffer_size = sizeof(UboViewProjection);

/*  /// Left for Reference; we dont use Dynamic Uniform Buffers for our Model Matrix anymore.
    /// model_dynamicUniformBuffer size
    VkDeviceSize model_dynamicBuffer_size = modelUniformAlignment * MAX_OBJECTS;
*/    

    /// One uniform buffer for each image ( and by extension, command buffer)
    viewProjection_uniformBuffer.resize(swapChainImages.size());        /// Resize to have as many ViewProjection buffers as images in swapchain
    viewProjection_uniformBufferMemory.resize(swapChainImages.size());    
    viewProjection_uniformBufferMemory_info.resize(swapChainImages.size());    

/*  /// Left for Reference; we dont use Dynamic Uniform Buffers for our Model Matrix anymore.
    model_dynamicUniformBuffer.resize(swapChainImages.size());          /// Resize to have as many Model dynamic buffers as images in swapchain
    model_dynamicUniformBufferMemory.resize(swapChainImages.size());
*/    

    /// Create Uniform Buffers 
    for(size_t i = 0; i < swapChainImages.size(); i++)
    {
        /// Create regular Uniform Buffers
        vengine_helper::createBuffer(
            {
                .physicalDevice = this->mainDevice.physicalDevice, 
                .device         = this->mainDevice.logicalDevice, 
                .bufferSize     = viewProjection_buffer_size, 
                .bufferUsageFlags = vk::BufferUsageFlagBits::eUniformBuffer,         /// We're going to use this as a Uniform Buffer...
                // .bufferProperties = vk::MemoryPropertyFlagBits::eHostVisible         /// So we can access the Data from the HOST (CPU)
                //                     | vk::MemoryPropertyFlagBits::eHostCoherent,     /// So we don't have to flush the data constantly...
                .bufferProperties = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                                | VMA_ALLOCATION_CREATE_MAPPED_BIT,
                .buffer         = &this->viewProjection_uniformBuffer[i], 
                //.bufferMemory   = &this->viewProjection_uniformBufferMemory[i],
                .bufferMemory   = &this->viewProjection_uniformBufferMemory[i],
                .allocationInfo   = &this->viewProjection_uniformBufferMemory_info[i],
                .vma = &vma
            });

        registerVkObjectDbgInfo("ViewProjection UniformBuffer["+std::to_string(i)+"]", vk::ObjectType::eBuffer, reinterpret_cast<uint64_t>(vk::Buffer::CType(this->viewProjection_uniformBuffer[i])));
        //registerVkObjectDbgInfo("ViewProjection UniformBufferMemory["+std::to_string(i)+"]", vk::ObjectType::eDeviceMemory, reinterpret_cast<uint64_t>(vk::DeviceMemory::CType(this->viewProjection_uniformBufferMemory[i]))); (!!)

/*      /// Left for Reference; we dont use Dynamic Uniform Buffers for our Model Matrix anymore.
        /// Create Dynamic Uniform Bufferes
        vengine_helper::createBuffer(
            this->mainDevice.physicalDevice, 
            this->mainDevice.logicalDevice, 
            model_dynamicBuffer_size, 
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,         /// We're going to use this as a Dynamic Uniform Buffer, 
                                                        //// same flag as for any UniformBuffer...
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT         /// So we can access the Data from the HOST (CPU)
            | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,     /// So we don't have to flush the data constantly...
            &this->model_dynamicUniformBuffer[i], 
            &this->model_dynamicUniformBufferMemory[i]);
*/    
    }

}

void VulkanRenderer::createDescriptorPool()
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    /// - CRTEATE UNIFORM DESCRIPTOR POOL -

    /// Pool Size is definied by the Type of the Descriptors times number of those Descriptors
    /// viewProjection uniform Buffer Pool size
    vk::DescriptorPoolSize viewProjection_poolSize {};
    viewProjection_poolSize.setType(vk::DescriptorType::eUniformBuffer);                                     /// Descriptors in Set will be of Type Uniform Buffer    
    viewProjection_poolSize.setDescriptorCount(static_cast<uint32_t>(viewProjection_uniformBuffer.size())); /// How many Descriptors we want, we want One uniformBuffer so we its only the size of our uniformBuffer

/*  /// Left for Reference; we dont use Dynamic Uniform Buffers for our Model Matrix anymore.

    /// Model Dynamic uniform Buffer Pool size
    VkDescriptorPoolSize model_poolSize{};
    model_poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;                            /// Dynamic Uniform Buffer Type!
    model_poolSize.descriptorCount = static_cast<uint32_t>(model_dynamicUniformBuffer.size());  /// how many descriptors we want...
*/

    std::vector<vk::DescriptorPoolSize> descriptorPoolSizes{
        viewProjection_poolSize
        /// Left for Reference; we dont use Dynamic Uniform Buffers for our Model Matrix anymore.
        ///,model_poolSize 
        
    };

    /// Data to create Descriptor Pool
    vk::DescriptorPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.setMaxSets(static_cast<uint32_t>(swapChainImages.size()));             /// Max Nr Of descriptor Sets that can be created from the pool, 
                                                                                        //// Same as the number of buffers / images we have. 
    poolCreateInfo.setPoolSizeCount(static_cast<uint32_t>(descriptorPoolSizes.size()));   /// Based on how many pools we have in our descriptorPoolSizes
    poolCreateInfo.setPPoolSizes(descriptorPoolSizes.data());                          /// PoolSizes to create the Descriptor Pool with

    this->descriptorPool = this->mainDevice.logicalDevice.createDescriptorPool(poolCreateInfo);
    registerVkObjectDbgInfo("DescriptorPool UniformBuffer ", vk::ObjectType::eDescriptorPool, reinterpret_cast<uint64_t>(vk::DescriptorPool::CType(this->descriptorPool)));
        
    /// - CRTEATE SAMPLER DESCRIPTOR POOL -
    /// Texture Sampler Pool
    vk::DescriptorPoolSize samplerPoolSize{};
    samplerPoolSize.setType(vk::DescriptorType::eCombinedImageSampler);       /// This descriptor pool will have descriptors for Image and Sampler combined    
                                                                            //// NOTE; Should be treated as seperate Concepts! but this will be enough...
    samplerPoolSize.setDescriptorCount(MAX_OBJECTS);                          /// There will be as many Descriptor Sets as there are Objects...
                                                                            ////NOTE; This WILL limit us to only have ONE texture per Object...

    vk::DescriptorPoolCreateInfo samplerPoolCreateInfo{};
    samplerPoolCreateInfo.setMaxSets(MAX_OBJECTS); 
    samplerPoolCreateInfo.setPoolSizeCount(uint32_t (1));
    samplerPoolCreateInfo.setPPoolSizes(&samplerPoolSize);
    /*/// NOTE; While the above code does work (The limit of SamplerDescriptorSets are alot higher than Descriptor Sets for Uniform Buffers)
                It's not the best solution. 
                The Correct way of doing this would be to take advantage of Array Layers and Texture Atlases.
                Right now we are taking up alot of unncessary memory by enabling us to create unncessary Descriptor Sets, 
                We would LIKE to limit the maxSets value to be as low as possible...
    */

    this->samplerDescriptorPool = this->mainDevice.logicalDevice.createDescriptorPool(samplerPoolCreateInfo);
    registerVkObjectDbgInfo("DescriptorPool ImageSampler ", vk::ObjectType::eDescriptorPool, reinterpret_cast<uint64_t>(vk::DescriptorPool::CType(this->samplerDescriptorPool)));


    /// - CREATE INPUT ATTTACHMENT DESCRIPTOR POOL -
    /// Color Attachment Pool Size
    vk::DescriptorPoolSize colorInputPoolSize{};
    colorInputPoolSize.setType(vk::DescriptorType::eInputAttachment);                          /// Will be used by a Subpass as a Input Attachment... (??) 
    colorInputPoolSize.setDescriptorCount(static_cast<uint32_t>(colorBufferImageView.size()));/// One Descriptor per colorBufferImageView

    /// Depth attachment Pool Size
    vk::DescriptorPoolSize depthInputPoolSize{};
    depthInputPoolSize.setType(vk::DescriptorType::eInputAttachment);                          /// Will be used by a Subpass as a Input Attachment... (??) 
    depthInputPoolSize.descriptorCount = static_cast<uint32_t>(depthBufferImageView.size());/// One Descriptor per depthBufferImageView

    std::array<vk::DescriptorPoolSize, 2> inputPoolSizes {
        colorInputPoolSize,
        depthInputPoolSize
    };

    /// Create Input attachment pool
    vk::DescriptorPoolCreateInfo inputPoolCreateInfo {};
    inputPoolCreateInfo.setMaxSets(static_cast<uint32_t>(swapChainImages.size()));
    inputPoolCreateInfo.setPoolSizeCount(static_cast<uint32_t>(inputPoolSizes.size()));
    inputPoolCreateInfo.setPPoolSizes(inputPoolSizes.data());
    inputPoolCreateInfo.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet); // In order to be able to free this set later (recreate swapchain)

    this->inputDescriptorPool = this->mainDevice.logicalDevice.createDescriptorPool(inputPoolCreateInfo);
    registerVkObjectDbgInfo("DescriptorPool InputAttachment ", vk::ObjectType::eDescriptorPool, reinterpret_cast<uint64_t>(vk::DescriptorPool::CType(this->inputDescriptorPool)));

}

void VulkanRenderer::allocateDescriptorSets(){
    /// Resize Descriptor Set; one Descriptor Set per UniformBuffer
    descriptorSets.resize(swapChainImages.size()); /// Since we have a uniform buffer per images, better use size of swapchainImages!

    /// Copy our DescriptorSetLayout so we have one per Image (one per UniformBuffer)
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts(swapChainImages.size(),this->descriptorSetLayout);

    /// Descriptor Set Allocation Info
    vk::DescriptorSetAllocateInfo setAllocInfo;
    setAllocInfo.setDescriptorPool(this->descriptorPool);                                   /// Pool to allocate descriptors (Set?) from   
    setAllocInfo.setDescriptorSetCount(static_cast<uint32_t>(this->swapChainImages.size()));/// One Descriptor set per Image 
                                                                                        //// (Same as uniformBuffers since we have One uniformBuffer per Image...)
    setAllocInfo.setPSetLayouts(descriptorSetLayouts.data());                               /// Layouts to use to allocate sets (1:1 relationship)

    /// Allocate all descriptor sets
    this->descriptorSets = this->mainDevice.logicalDevice.allocateDescriptorSets(setAllocInfo);
}

void VulkanRenderer::createDescriptorSets()
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    /// Resize Descriptor Set; one Descriptor Set per UniformBuffer
    descriptorSets.resize(swapChainImages.size()); /// Since we have a uniform buffer per images, better use size of swapchainImages!

    
    for(size_t i = 0; i < swapChainImages.size();i++)
    {
        //registerVkObjectDbgInfo("DescriptorSet["+std::to_string(i)+"]  UniformBuffer", VK_OBJECT_TYPE_DESCRIPTOR_SET, reinterpret_cast<uint64_t>(this->descriptorSets[i]));
        registerVkObjectDbgInfo("DescriptorSet["+std::to_string(i)+"]  UniformBuffer", vk::ObjectType::eDescriptorSet, reinterpret_cast<uint64_t>(vk::DescriptorSet::CType(this->descriptorSets[i])));
    }

    /// Update all of the Descriptor Set buffer binding
    for(size_t i = 0; i < this->swapChainImages.size(); i++)
    {
        /// - VIEW PROJECTION DESCRIPTOR - 
        /// Describe the Buffer info and Data offset Info
        vk::DescriptorBufferInfo viewProjection_BufferInfo; 
        viewProjection_BufferInfo.setBuffer(this->viewProjection_uniformBuffer[i]);/// Buffer to get the Data from
        viewProjection_BufferInfo.setOffset(0);                                    /// Position Of start of Data; 
                                                                                 //// Offset from the start (0), since we want to write all data
        viewProjection_BufferInfo.setRange(sizeof(UboViewProjection));             /// Size of data ... 

        /// Data to describe the connection between Binding and Uniform Buffer
        vk::WriteDescriptorSet viewProjection_setWrite;
        viewProjection_setWrite.setDstSet(this->descriptorSets[i]);              /// Descriptor Set to update
        viewProjection_setWrite.setDstBinding(uint32_t (0));                                    /// Binding to update (Matches with Binding on Layout/Shader)
        viewProjection_setWrite.setDstArrayElement(uint32_t (0));                                /// Index in array we want to update (if we use an array, we do not. thus 0)
        viewProjection_setWrite.setDescriptorType(vk::DescriptorType::eUniformBuffer);/// Type of Descriptor
        viewProjection_setWrite.setDescriptorCount(uint32_t (1));                                /// Amount of Descriptors to update
        viewProjection_setWrite.setPBufferInfo(&viewProjection_BufferInfo); 

/*      /// Left for Reference; we dont use Dynamic Uniform Buffers for our Model Matrix anymore.
        /// - MODEL DESCRIPTOR - 
        /// Model Dynamic Buffer binding info
        VkDescriptorBufferInfo model_dynamicBuffer_info{};
        model_dynamicBuffer_info.buffer = this->model_dynamicUniformBuffer[i];
        model_dynamicBuffer_info.offset = 0;                        /// Start of the first piece of data in the Dynamic Buffer
        model_dynamicBuffer_info.range  = modelUniformAlignment;    /// Range of a alignment

        /// Data to describe the connection between Binding and Dynamic Uniform Buffer
        VkWriteDescriptorSet model_setWrite{};
        model_setWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        model_setWrite.dstSet           = this->descriptorSets[i];
        model_setWrite.dstBinding       = 1; 
        model_setWrite.dstArrayElement  = 0;
        model_setWrite.descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        model_setWrite.descriptorCount  = 1;
        model_setWrite.pBufferInfo      = &model_dynamicBuffer_info;

*/
        /// List of descriptorSetWrites
        std::vector<vk::WriteDescriptorSet> descriptorSetWrites{
            viewProjection_setWrite
            /// Left for Reference; we dont use Dynamic Uniform Buffers for our Model Matrix anymore.
            ///,model_setWrite
        };

        /// Update the Descriptor Set with new buffer/binding info
        this->mainDevice.logicalDevice.updateDescriptorSets(
            viewProjection_setWrite,  /// Update all Descriptor sets in descripotrSetWrites vector
            nullptr
        );
    }

}

void VulkanRenderer::createInputDescriptorSets()
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    /// Resize array to hold Descriptor Set for each Swap Chain image
    this->inputDescriptorSets.resize(swapChainImages.size());

    /// Fill vector of layouts ready for set creation
    std::vector<vk::DescriptorSetLayout> inputSetLayouts(this->swapChainImages.size(), this->inputSetLayout);

    /// Input Attachment Descriptor Set Allocation Info
    vk::DescriptorSetAllocateInfo inputSetAllocInfo;
    inputSetAllocInfo.setDescriptorPool(this->inputDescriptorPool);
    inputSetAllocInfo.setDescriptorSetCount(static_cast<uint32_t>(swapChainImages.size())); /// One descriptor per image in swapchain
    inputSetAllocInfo.setPSetLayouts(inputSetLayouts.data());    

    /// Allocate Descriptor Sets
    this->inputDescriptorSets = this->mainDevice.logicalDevice.allocateDescriptorSets(inputSetAllocInfo);  

    for(size_t i = 0; i < swapChainImages.size();i++)
    {
        //registerVkObjectDbgInfo("DescriptorSet["+std::to_string(i)+"]  InputAttachment", VK_OBJECT_TYPE_DESCRIPTOR_SET, reinterpret_cast<uint64_t>(this->inputDescriptorSets[i]));
        registerVkObjectDbgInfo("DescriptorSet["+std::to_string(i)+"] InputAttachment", vk::ObjectType::eDescriptorSet, reinterpret_cast<uint64_t>(vk::DescriptorSet::CType(this->inputDescriptorSets[i])));
    }

    for(size_t i = 0; i < swapChainImages.size(); i++)
    {
        /// Color Attachment Descriptor
        vk::DescriptorImageInfo colorAttachmentDescriptor;
        colorAttachmentDescriptor.setImageLayout(vk::ImageLayout::eReadOnlyOptimal);   /// Layout of the Image when it will be Read from! (This is what we've setup the second subpass to expect...)
        colorAttachmentDescriptor.setImageView(this->colorBufferImageView[i]);
        colorAttachmentDescriptor.setSampler(VK_NULL_HANDLE);                             /// A Subpass Input Attachment can't have a sampler! 

        /// Color Attachment Descriptor Write
        vk::WriteDescriptorSet colorWrite;
        colorWrite.setDstSet(this->inputDescriptorSets[i]);
        colorWrite.setDstBinding(uint32_t (0)); 
        colorWrite.setDstArrayElement(uint32_t (0));
        colorWrite.setDescriptorType(vk::DescriptorType::eInputAttachment);
        colorWrite.setDescriptorCount(uint32_t(1));
        colorWrite.setPImageInfo(&colorAttachmentDescriptor);

        
        /// Depth Attachment Descriptor
        vk::DescriptorImageInfo depthAttachmentDescriptor;
        depthAttachmentDescriptor.setImageLayout(vk::ImageLayout::eReadOnlyOptimal);   /// Layout of the Image when it will be Read from! (This is what we've setup the second subpass to expect...)
        depthAttachmentDescriptor.setImageView(this->depthBufferImageView[i]);
        depthAttachmentDescriptor.setSampler(VK_NULL_HANDLE);                             /// A Subpass Input Attachment can't have a sampler! 

        /// Depth Attachment Descriptor Write
        vk::WriteDescriptorSet depthWrite;
        depthWrite.setDstSet(this->inputDescriptorSets[i]);
        depthWrite.setDstBinding(uint32_t (1)); 
        depthWrite.setDstArrayElement(uint32_t (0));
        depthWrite.setDescriptorType(vk::DescriptorType::eInputAttachment);
        depthWrite.setDescriptorCount(uint32_t (1));
        depthWrite.setPImageInfo(&depthAttachmentDescriptor);

        /// List of the Input Descriptor Sets Writes 
        std::array<vk::WriteDescriptorSet,2> inputDescriptorSetWrites{
            colorWrite,
            depthWrite
        };

        this->mainDevice.logicalDevice.updateDescriptorSets(
            inputDescriptorSetWrites,
            0
        );
    }
    
}

void VulkanRenderer::createCommandPool(vk::CommandPool& commandPool, vk::CommandPoolCreateFlags flags, std::string&& name = "NoName")
{
    vk::CommandPoolCreateInfo commandPoolCreateInfo; 
    commandPoolCreateInfo.setFlags(flags);
    commandPoolCreateInfo.setQueueFamilyIndex(this->QueueFamilies.graphicsFamily);
    commandPool = this->mainDevice.logicalDevice.createCommandPool(commandPoolCreateInfo);
    registerVkObjectDbgInfo(name, vk::ObjectType::eCommandPool, reinterpret_cast<uint64_t>(vk::CommandPool::CType(commandPool)));

    ///TODO: automatic trash collection
}

void VulkanRenderer::createCommandBuffer(vk::CommandBuffer& commandBuffer,vk::CommandPool& commandPool, std::string&&  name = "NoName")
{
    vk::CommandBufferAllocateInfo commandBuffferAllocationInfo{};
    commandBuffferAllocationInfo.setCommandPool(commandPool);
    commandBuffferAllocationInfo.setLevel(vk::CommandBufferLevel::ePrimary);
    commandBuffferAllocationInfo.setCommandBufferCount(uint32_t(1));
    
    commandBuffer = this->mainDevice.logicalDevice.allocateCommandBuffers(commandBuffferAllocationInfo)[0];
    registerVkObjectDbgInfo(name, vk::ObjectType::eCommandBuffer, reinterpret_cast<uint64_t>(vk::CommandBuffer::CType(commandBuffer)));
    ///TODO: automatic trash collection    
}

void VulkanRenderer::createFrameBuffer(vk::Framebuffer& frameBuffer,std::vector<vk::ImageView>& attachments,vk::RenderPass& renderPass, vk::Extent2D& extent, std::string&& name = "NoName")
{
    vk::FramebufferCreateInfo framebufferCreateInfo{};
    framebufferCreateInfo.setRenderPass(renderPass);
    framebufferCreateInfo.setAttachmentCount(static_cast<uint32_t>(attachments.size()));
    framebufferCreateInfo.setPAttachments(attachments.data());
    framebufferCreateInfo.setWidth(extent.width);
    framebufferCreateInfo.setHeight(extent.height);
    framebufferCreateInfo.setLayers(uint32_t(1));

    frameBuffer = mainDevice.logicalDevice.createFramebuffer(framebufferCreateInfo);
    registerVkObjectDbgInfo(name, vk::ObjectType::eFramebuffer, reinterpret_cast<uint64_t>(vk::Framebuffer::CType(frameBuffer)));
}

void VulkanRenderer::updateUniformBuffers(uint32_t imageIndex)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    /// - REGULAR UNIFORM BUFFER - 
    /// - Copy View Projection   - 

    void * data = nullptr;
    
    vmaMapMemory(this->vma, this->viewProjection_uniformBufferMemory[imageIndex], &data);
    //memcpy(this->viewProjection_uniformBufferMemory_info[imageIndex].pMappedData, &uboViewProjection, sizeof(UboViewProjection));
    memcpy(data, &uboViewProjection, sizeof(UboViewProjection));
    vmaUnmapMemory(this->vma, this->viewProjection_uniformBufferMemory[imageIndex]);


/*  /// Following Code is left for Rerference of how to update a Dynamic Uniform Buffer... 
    /// - DYNAMIC UNIFORM BUFFER - 
    /// - Copy Model data        - 
    for(size_t i = 0; i < meshes.size(); i++)
    {
        /// thisModel will refer to the address within the modelTransferSpace that belongs to meshs[i] model
        auto* thisModel = (Model*)((uint64_t)modelTransferSpace + (i * modelUniformAlignment));
        *thisModel = meshes[i].getModel();   /// Update the data in the allocated memory for this specific model/mesh
    }
    vkMapMemory(this->mainDevice.logicalDevice, 
        this->model_dynamicUniformBufferMemory[imageIndex], /// Memory that corresponds to this VkDeviceMemory
        0,                                                  /// Ofset into the memory
        modelUniformAlignment * meshes.size(),              /// size we want to map, i.e. size of all our meshes
        0,                                                  /// flags; we use non
        &data);                                             /// The handle that will point to the GPU memory (??)
    /// Copy the updated values from mvp to the data on the GPU! 
    memcpy(data, 
        modelTransferSpace,                     /// Note, that we use the allocated memory in modelTransferSpace! 
        modelUniformAlignment * meshes.size()); /// We want to copy for all our meshes
    /// Unmap to confirm the copy (??)
    vkUnmapMemory(this->mainDevice.logicalDevice, this->model_dynamicUniformBufferMemory[imageIndex]);
*/
    
}

void VulkanRenderer::updateUBO_camera_Projection()
{
    using namespace vengine_helper::config;
    uboViewProjection.projection  = glm::perspective(                               /// The Camera Projection Perspective
                            //glm::radians(v_conf::camera_fov()),                               /// View Angle in the y-axis
                            //glm::radians(CAMERA_FOV),                                         /// View Angle in the y-axis
                            glm::radians(DEF<float>(CAM_FOV)),                               /// View Angle in the y-axis
                            (float)swapChainExtent.width/(float)swapChainExtent.height,         /// Setting up the Aspect Ratio
                            DEF<float>(CAM_NP),                                              /// The Near Plane
                            DEF<float>(CAM_FP));                                             /// The Far Plane
    uboViewProjection.projection[1][1] *= -1;     /// Since GLM is made for OpenGL and OpenGL uses RightHanded system; Positive Y is considered the Up dir
}

void VulkanRenderer::updateUBO_camera_view(glm::vec3 eye, glm::vec3 center, glm::vec3 up)
{
    uboViewProjection.view = glm::lookAt(
                            eye,            /// Eye   : Where the Camera is positioned in the world
                            center,         /// Target: Point the Camera is looking at
                            up);            /// Up    : Up direction 
}

void VulkanRenderer::recordRenderPassCommands_imgui(uint32_t currentImageIndex)
{
    #ifndef VENGINE_NO_PROFILING
    //ZoneScoped; //:NOLINT     
    ZoneTransient(recordRenderPassCommands_imgui_zone1,  true); //:NOLINT   
    #endif

    this->mainDevice.logicalDevice.resetCommandPool(this->commandPools_imgui[currentImageIndex]);

    vk::CommandBufferBeginInfo commandBufferBeginInfo{};
    commandBufferBeginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    this->commandBuffers_imgui[currentImageIndex].begin(commandBufferBeginInfo);

    static const vk::ClearColorValue  clear_black(std::array<float,4> {0.F, 0.F, 0.F, 1.F});  
    static const vk::ClearValue  clearValue {clear_black};  
    vk::RenderPassBeginInfo renderPassBeginInfo{};
    vk::SubpassBeginInfo subpassBeginInfo;
    subpassBeginInfo.setContents(vk::SubpassContents::eInline);
    renderPassBeginInfo.setRenderPass(this->renderPass_imgui);
    renderPassBeginInfo.renderArea.setExtent(this->swapChainExtent);
    renderPassBeginInfo.renderArea.setOffset(vk::Offset2D(0, 0));
    renderPassBeginInfo.setFramebuffer(this->frameBuffers_imgui[currentImageIndex]);
    renderPassBeginInfo.setPClearValues(&clearValue);         /// List of clear values
    renderPassBeginInfo.setClearValueCount(uint32_t(1));

    this->commandBuffers_imgui[currentImageIndex].beginRenderPass2(&renderPassBeginInfo, &subpassBeginInfo);

    vk::Viewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) swapChainExtent.width;
    viewport.height = (float) swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    this->commandBuffers_imgui[currentImageIndex].setViewport(0, 1, &viewport);

    vk::Rect2D scissor{};
    scissor.offset = vk::Offset2D{0, 0};
    scissor.extent = swapChainExtent;
    this->commandBuffers_imgui[currentImageIndex].setScissor( 0, 1, &scissor);

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), this->commandBuffers_imgui[currentImageIndex]);

    vk::SubpassEndInfo imgui_subpassEndInfo;                            
    commandBuffers_imgui[currentImageIndex].endRenderPass2(imgui_subpassEndInfo);
    this->commandBuffers_imgui[currentImageIndex].end();
}

void VulkanRenderer::recordRenderPassCommands_Base(uint32_t currentImageIndex) 
{
#ifndef VENGINE_NO_PROFILING
    //ZoneScoped; //:NOLINT     
    ZoneTransient(recordRenderPassCommands_zone1,  true); //:NOLINT   
#endif
    /// Information about how to befin each Command Buffer...

    vk::CommandBufferBeginInfo bufferBeginInfo;
    bufferBeginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    
    /// Following flag is now irrellevant, since we use Fences when we draw! (??) TODO: Check if the previous statement is true... (validation layers seems to tell something else)
    ////bufferBeginInfo.vk::CommandBufferUsageFlagBits::eSimultaneousUse;  /// Buffer can be resubmitted when it has already been submitted and is awaiting execution
                                                                             /// Defines if we can have two of the same commandbuffer on a queue at the same time: this flag means YES

    /// Information about how to begin a render pass (Only needed for graphical applications)
    vk::RenderPassBeginInfo renderPassBeginInfo;
    renderPassBeginInfo.setRenderPass(this->renderPass_base);                      /// Render Pass to Begin
    renderPassBeginInfo.renderArea.setOffset(vk::Offset2D(0, 0));                 /// Start of render pass (in pixels...)
    renderPassBeginInfo.renderArea.setExtent(this->swapChainExtent);          /// Size of region to run render pass on (starting at offset)
     
    static const vk::ClearColorValue  clear_black(std::array<float,4> {0.F, 0.F, 0.F, 1.F});    
    static const vk::ClearColorValue  clear_Plum (std::array<float,4> {221.F/256.0F, 160.F/256.0F, 221.F/256.0F, 1.0F});

    std::array<vk::ClearValue, 3> clearValues = {        /// Clear values consists of a VkClearColorValue and a VkClearDepthStencilValue

            vk::ClearValue(                              //// of type VkClearColorValue 
                vk::ClearColorValue{ clear_black}     /// Clear Value for Attachment 0
            ),  
            vk::ClearValue(                              //// of type VkClearColorValue 
                vk::ClearColorValue{clear_Plum}     /// Clear Value for Attachment 1
            ),            
            vk::ClearValue(                              /// Clear Value for Attachment 2
                vk::ClearDepthStencilValue(
                    1.F,    // depth
                    0       // stencil
                )
            )
    };

    renderPassBeginInfo.setPClearValues(clearValues.data());         /// List of clear values
    renderPassBeginInfo.setClearValueCount(static_cast<uint32_t>(clearValues.size()));

    /// The only changes we do per commandbuffer is to change the swapChainFrameBuffer a renderPass should point to...
    renderPassBeginInfo.setFramebuffer(swapChainFrameBuffers[currentImageIndex]);

    vk::SubpassBeginInfoKHR subpassBeginInfo;
    subpassBeginInfo.setContents(vk::SubpassContents::eInline);
    
    
    /// Start recording commands to commandBuffer!
    commandBuffers[currentImageIndex].begin(bufferBeginInfo);
    {   /// Scope for Tracy Vulkan Zone...
        #ifndef VENGINE_NO_PROFILING
        TracyVkZone(
            this->tracyContext[currentImageIndex],
            this->commandBuffers[currentImageIndex],"Render Record Commands");
        #endif
        {
        #ifndef VENGINE_NO_PROFILING        
        ZoneTransient(recordRenderPassCommands_zone2,  true); //:NOLINT   
        #endif
        
        #pragma region commandBufferRecording
            /// Begin Render Pass!    
            /// vk::SubpassContents::eInline; all the render commands themselves will be primary render commands (i.e. will not use secondary commands buffers)
            commandBuffers[currentImageIndex].beginRenderPass2(renderPassBeginInfo, subpassBeginInfo);            

                /// Bind Pipeline to be used in render pass
                commandBuffers[currentImageIndex].bindPipeline(vk::PipelineBindPoint::eGraphics, this->graphicsPipeline);
                /// vk::PipelineBindPoint::eGraphics: What Kind of pipeline we are binding...

                /// For every Mesh we have
                for(auto & currModel : modelList)
                {
                    auto modelMatrix= currModel.getModelMatrix();
                    /// "Push" Constants to given Shader Stage Directly (using no Buffer...)
                    this->commandBuffers[currentImageIndex].pushConstants(
                        this->pipelineLayout,
                        vk::ShaderStageFlagBits::eVertex,   /// Stage to push the Push Constant to.
                        uint32_t(0),                        /// Offset of Push Constants to update; 
                                                            //// Offset into the Push Constant Block (if more values are used (??))
                        sizeof(modelMatrix),                /// Size of data being pushed
                        &modelMatrix                        /// Actual data being pushed (can also be an array)
                    );

                    for(auto& modelPart : currModel.getModelParts())
                    {
                        /// -- BINDING VERTEX BUFFERS --
                        //std::array<vk::Buffer,1> vertexBuffer = { currModel.getMesh(k)->getVertexBuffer()};                /// Buffers to bind
                        std::array<vk::Buffer,1> vertexBuffer = { modelPart.second.vertexBuffer};                /// Buffers to bind
                        std::array<vk::DeviceSize,1> offsets  = {0};                                           /// Offsets into buffers being bound
                        commandBuffers[currentImageIndex].bindVertexBuffers2(
                            uint32_t(0),
                            uint32_t(1),
                            vertexBuffer.data(),
                            offsets.data(),
                            nullptr,        ///NOTE: Could also be a pointer to an array of the size in bytes of vertex data bound from pBuffers (vertexBuffer)
                            nullptr         ///NOTE: Could also be a pointer to an array of buffer strides
                        );
                        /// Bind Mesh Index Buffer; Define the Index Buffer that decides how to draw the Vertex Buffers
                        commandBuffers[currentImageIndex].bindIndexBuffer(
                            //currModel.getMesh(k)->getIndexBuffer(), 
                            modelPart.second.indexBuffer, 
                            0,
                            vk::IndexType::eUint32);
                        
                    /*  /// Left for Reference, we dont use Dynamic Uniform Buffers for our Model anymore...                                    
                        /// Dynamic Uniform Buffer Offset Amount
                        uint32_t dynamicUniformBuffer_offset = static_cast<uint32_t>(modelUniformAlignment) * j; /// Will give us the correct position in the memory!
                    */
                        /// We're going to bind Two descriptorSets! put them in array...
                        std::array<vk::DescriptorSet,2> descriptorSetGroup{
                            this->descriptorSets[currentImageIndex],                /// Use the descriptor set for the Image                            
                            this->samplerDescriptorSets[ modelPart.second.textureID]   /// Use the Texture which the current mesh has
                        };
                        /// Bind Descriptor Sets; this will be the binging for both the Dynamic Uniform Buffers and the non dynamic...
                        this->commandBuffers[currentImageIndex].bindDescriptorSets(
                            vk::PipelineBindPoint::eGraphics, /// The descriptor set can be used at ANY stage of the Graphics Pipeline
                            this->pipelineLayout,            /// The Pipeline Layout that describes how the data will be accessed in our shaders
                            0,                               /// Which Set is the first we want to use? We want to use the First set (thus 0)
                            static_cast<uint32_t>(descriptorSetGroup.size()),/// How many Descriptor Sets where going to go through? DescriptorSet for View and Projection, and one for Texture
                            descriptorSetGroup.data(),                       /// The Descriptor Set to be used (Remember, 1:1 relationship with CommandBuffers/Images)
                            0,                               /// Dynamic Offset Count;  we dont Dynamic Uniform Buffers use anymore...
                            nullptr);                        /// Dynamic Offset;        We dont use Dynamic Uniform Buffers  anymore...

                        /*///Left for Reference, (This was the two last parameters of vkCmdBindDescriptorSets function)
                            1,                               /// Dynamic Offset Count; 
                            &dynamicUniformBuffer_offset);    /// Dynamic Offset
                        */
                        vk::Viewport viewport{};
                        viewport.x = 0.0f;
                        viewport.y = 0.0f;
                        viewport.width = (float) swapChainExtent.width;
                        viewport.height = (float) swapChainExtent.height;
                        viewport.minDepth = 0.0f;
                        viewport.maxDepth = 1.0f;
                        this->commandBuffers[currentImageIndex].setViewport(0, 1, &viewport);

                        vk::Rect2D scissor{};
                        scissor.offset = vk::Offset2D{0, 0};
                        scissor.extent = swapChainExtent;
                        this->commandBuffers[currentImageIndex].setScissor( 0, 1, &scissor);
                        /// Execute Pipeline!
                        this->commandBuffers[currentImageIndex].drawIndexed(                            
                            modelPart.second.indexCount,  /// Number of vertices to draw (nr of indexes)
                            1,                          /// We're drawing only one instance
                            0,                          /// Start at index 0
                            0,                          /// Vertex offset is 0, i.e. no offset! 
                            0);                         /// We Draw Only one Instance, so first will be 0...
                        /*
                        /// Old way of drawing, Here we dont draw based on the indicies!
                        vkCmdDraw(commandBuffers[i],
                                static_cast<uint32_t>(firstMesh.getVertexCount()),    /// Amount of vertices we want to draw, i.e. how many drawcalls... (gl_VertexIndex)
                                1,    /// how many instances of the object
                                0,    /// First vertex is 0
                                0);   /// we only have 1 instance, so the first is on index 0...
                        */
                    }                

                                 
                }

                /// Start Second Subpass
                vk::SubpassEndInfo subpassEndInfo;                
                commandBuffers[currentImageIndex].nextSubpass2(subpassBeginInfo,subpassEndInfo);

                this->commandBuffers[currentImageIndex].bindPipeline(vk::PipelineBindPoint::eGraphics, this->secondGraphicsPipeline);

                this->commandBuffers[currentImageIndex].bindDescriptorSets(
                    vk::PipelineBindPoint::eGraphics,
                    this->secondPipelineLayout,
                    uint32_t(0),
                    uint32_t(1),
                    &this->inputDescriptorSets[currentImageIndex],
                    uint32_t(0),
                    nullptr
                );


                  vk::Viewport viewport{};
                    viewport.x = 0.0f;
                    viewport.y = 0.0f;
                    viewport.width = (float) swapChainExtent.width;
                    viewport.height = (float) swapChainExtent.height;
                    viewport.minDepth = 0.0f;
                    viewport.maxDepth = 1.0f;
                    this->commandBuffers[currentImageIndex].setViewport(0, 1, &viewport);

                    vk::Rect2D scissor{};
                    scissor.offset = vk::Offset2D{0, 0};
                    scissor.extent = swapChainExtent;
                    this->commandBuffers[currentImageIndex].setScissor( 0, 1, &scissor);

            
                this->commandBuffers[currentImageIndex].draw(
                    3,      /// We will draw a Triangle, so we only want to draw 3 vertices
                    1,
                    0,
                    0);

            
            /// End Render Pass!
            commandBuffers[currentImageIndex].endRenderPass2(subpassEndInfo);
            /*!CMD in a function means that the function is something that is being recorded!
                * */
        }
        #pragma endregion commandBufferRecording
        
        #ifndef VENGINE_NO_PROFILING
        TracyVkCollect(this->tracyContext[currentImageIndex],
            this->commandBuffers[currentImageIndex]);
        #endif
    }
    /// Stop recording to a command buffer
    commandBuffers[currentImageIndex].end();

}

void VulkanRenderer::recordDynamicRenderingCommands(uint32_t currentImageIndex) 
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    /// Information about how to begin each Command Buffer...
    vk::CommandBufferBeginInfo bufferBeginInfo;
    bufferBeginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);    
     
    static const vk::ClearColorValue  clear_black(std::array<float,4> {0.F, 0.F, 0.F, 1.F});    
    static const vk::ClearColorValue  clear_Plum (std::array<float,4> {221.F/256.0F, 160.F/256.0F, 221.F/256.0F, 1.0F});

    vk::RenderingAttachmentInfo color_attachment_info;         
    color_attachment_info.imageView = this->swapChainImages[currentImageIndex].imageView;
    color_attachment_info.setImageLayout(vk::ImageLayout::eAttachmentOptimal);
    color_attachment_info.setLoadOp(vk::AttachmentLoadOp::eClear);
    color_attachment_info.setStoreOp(vk::AttachmentStoreOp::eStore);
    color_attachment_info.clearValue = clear_black;

    vk::RenderingAttachmentInfo depth_attachment_info;         
    depth_attachment_info.imageView = this->depthBufferImageView[currentImageIndex];
    depth_attachment_info.setImageLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
    depth_attachment_info.setLoadOp(vk::AttachmentLoadOp::eClear);
    depth_attachment_info.setStoreOp(vk::AttachmentStoreOp::eStore);
    depth_attachment_info.clearValue = vk::ClearValue(                              
                vk::ClearDepthStencilValue(
                    1.F,        // depth
                    0       // stencil
                )
            );
    
    vk::RenderingInfo renderingInfo{};
    renderingInfo.setFlags(vk::RenderingFlags());
    renderingInfo.setRenderArea({ vk::Offset2D(0, 0),  this->swapChainExtent});
    renderingInfo.setLayerCount(uint32_t (1));    
    renderingInfo.setColorAttachmentCount(uint32_t (1));
    renderingInfo.setPColorAttachments(&color_attachment_info);
    renderingInfo.setPDepthAttachment(&depth_attachment_info);
    renderingInfo.setPStencilAttachment(&depth_attachment_info);

    vk::SubpassBeginInfoKHR subpassBeginInfo;
    subpassBeginInfo.setContents(vk::SubpassContents::eInline);
    
    /// Start recording commands to commandBuffer!
    commandBuffers[currentImageIndex].begin(bufferBeginInfo);    
    {   /// Scope for Tracy Vulkan Zone...
    #ifndef VENGINE_NO_PROFILING
        TracyVkZone(
            this->tracyContext[currentImageIndex],
            this->commandBuffers[currentImageIndex],"Render Record Commands");
    #endif 

        vengine_helper::insertImageMemoryBarrier(
        createImageBarrierData{
            .cmdBuffer = commandBuffers[currentImageIndex],
            .image = this->swapChainImages[currentImageIndex].image,            
            .srcAccessMask = vk::AccessFlags2(),
            .dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
            .oldLayout = vk::ImageLayout::eUndefined,
            .newLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .srcStageMask = vk::PipelineStageFlagBits2::eTopOfPipe,
            .dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            .imageSubresourceRange = vk::ImageSubresourceRange{
                vk::ImageAspectFlagBits::eColor, 
                0,
                1,
                0,
                1}
            }
        );

        vengine_helper::insertImageMemoryBarrier(
            createImageBarrierData{
                .cmdBuffer = commandBuffers[currentImageIndex],
                .image = this->depthBufferImage[currentImageIndex],
                .srcAccessMask = vk::AccessFlags2(),
                .dstAccessMask = vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
                .oldLayout = vk::ImageLayout::eUndefined,
                .newLayout = vk::ImageLayout::eDepthAttachmentOptimal,
                .srcStageMask = vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
                .dstStageMask = vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
                .imageSubresourceRange = vk::ImageSubresourceRange{
                    vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 
                    0,
                    1,
                    0,
                    1}
            }
        );
            
            /// Begin DynamicRendering!      
            commandBuffers[currentImageIndex].beginRendering(renderingInfo);

                /// Bind Pipeline to be used in the DynamicRendering command
                commandBuffers[currentImageIndex].bindPipeline(vk::PipelineBindPoint::eGraphics, this->graphicsPipeline);

                #pragma region commandBufferRecording
                {
                    #ifndef VENGINE_NO_PROFILING
                    ZoneNamedN(loop_all_models, "loop_all_models", true);
                    #endif 
                
                    /// For every Mesh we have
                    for(auto & currModel : modelList)
                    {
                        {
                            #ifndef VENGINE_NO_PROFILING
                            ZoneNamedN(loop_per_model, "loop_per_model", true);
                            #endif 

                            auto modelMatrix= currModel.getModelMatrix();

                            /// "Push" Constants to given Shader Stage Directly (using no Buffer...)
                            this->commandBuffers[currentImageIndex].pushConstants(
                                this->pipelineLayout,
                                vk::ShaderStageFlagBits::eVertex,   /// Stage to push the Push Constant to.
                                uint32_t(0),                        /// Offset of Push Constants to update; 
                                                                    //// Offset into the Push Constant Block (if more values are used (??))
                                sizeof(modelMatrix),                /// Size of data being pushed
                                &modelMatrix                        /// Actual data being pushed (can also be an array)
                            );

                            for(auto& modelPart : currModel.getModelParts())
                            {
                                {
                                    #ifndef VENGINE_NO_PROFILING
                                    ZoneNamedN(loop_per_model_part, "loop_per_model_part", true);
                                    #endif 
                                    /// -- BINDING VERTEX BUFFERS --                                    
                                    std::array<vk::Buffer,1> vertexBuffer = { modelPart.second.vertexBuffer};                /// Buffers to bind
                                    std::array<vk::DeviceSize,1> offsets  = {0};                                           /// Offsets into buffers being bound
                                    commandBuffers[currentImageIndex].bindVertexBuffers2(
                                        uint32_t(0),
                                        uint32_t(1),
                                        vertexBuffer.data(),
                                        offsets.data(),
                                        nullptr,        ///NOTE: Could also be a pointer to an array of the size in bytes of vertex data bound from pBuffers (vertexBuffer)
                                        nullptr         ///NOTE: Could also be a pointer to an array of buffer strides
                                    );
                                    /// Bind Mesh Index Buffer; Define the Index Buffer that decides how to draw the Vertex Buffers
                                    commandBuffers[currentImageIndex].bindIndexBuffer(
                                        modelPart.second.indexBuffer, 
                                        0,
                                        vk::IndexType::eUint32);
                                    
                                /*  /// Left for Reference, we dont use Dynamic Uniform Buffers for our Model anymore...                                    
                                    /// Dynamic Uniform Buffer Offset Amount
                                    uint32_t dynamicUniformBuffer_offset = static_cast<uint32_t>(modelUniformAlignment) * j; /// Will give us the correct position in the memory!
                                */
                                    /// We're going to bind Two descriptorSets! put them in array...
                                    std::array<vk::DescriptorSet,2> descriptorSetGroup{
                                        this->descriptorSets[currentImageIndex],                /// Use the descriptor set for the Image                            
                                        this->samplerDescriptorSets[ modelPart.second.textureID]   /// Use the Texture which the current mesh has
                                    };
                                    /// Bind Descriptor Sets; this will be the binging for both the Dynamic Uniform Buffers and the non dynamic...
                                    this->commandBuffers[currentImageIndex].bindDescriptorSets(
                                        vk::PipelineBindPoint::eGraphics, /// The descriptor set can be used at ANY stage of the Graphics Pipeline
                                        this->pipelineLayout,            /// The Pipeline Layout that describes how the data will be accessed in our shaders
                                        0,                               /// Which Set is the first we want to use? We want to use the First set (thus 0)
                                        static_cast<uint32_t>(descriptorSetGroup.size()),/// How many Descriptor Sets where going to go through? DescriptorSet for View and Projection, and one for Texture
                                        descriptorSetGroup.data(),                       /// The Descriptor Set to be used (Remember, 1:1 relationship with CommandBuffers/Images)
                                        0,                               /// Dynamic Offset Count;  we dont Dynamic Uniform Buffers use anymore...
                                        nullptr);                        /// Dynamic Offset;        We dont use Dynamic Uniform Buffers  anymore...

                                    /*///Left for Reference, (This was the two last parameters of vkCmdBindDescriptorSets function)
                                        1,                               /// Dynamic Offset Count; 
                                        &dynamicUniformBuffer_offset);    /// Dynamic Offset
                                    */

                                    /// Execute Pipeline!
                                    this->commandBuffers[currentImageIndex].drawIndexed(                            
                                        modelPart.second.indexCount,  /// Number of vertices to draw (nr of indexes)
                                        1,                          /// We're drawing only one instance
                                        0,                          /// Start at index 0
                                        0,                          /// Vertex offset is 0, i.e. no offset! 
                                        0);                         /// We Draw Only one Instance, so first will be 0...
                                    /*
                                    /// Old way of drawing, Here we dont draw based on the indicies!
                                    vkCmdDraw(commandBuffers[i],
                                            static_cast<uint32_t>(firstMesh.getVertexCount()),    /// Amount of vertices we want to draw, i.e. how many drawcalls... (gl_VertexIndex)
                                            1,    /// how many instances of the object
                                            0,    /// First vertex is 0
                                            0);   /// we only have 1 instance, so the first is on index 0...
                                    */
                                }
                            }
                        }                                               
                    }
                }

            /// End DynamicRendering!            
            commandBuffers[currentImageIndex].endRendering();

        #pragma endregion commandBufferRecording
        vengine_helper::insertImageMemoryBarrier(
        createImageBarrierData{
            .cmdBuffer = commandBuffers[currentImageIndex],
            .image = this->swapChainImages[currentImageIndex].image,
            //.image = this->colorBufferImage[this->currentFrame],
            .srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
            .dstAccessMask = vk::AccessFlags2(),
            .oldLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .newLayout = vk::ImageLayout::ePresentSrcKHR,
            .srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            .dstStageMask = vk::PipelineStageFlagBits2::eBottomOfPipe,
            .imageSubresourceRange = vk::ImageSubresourceRange{
            vk::ImageAspectFlagBits::eColor, 
            0,
            1,
            0,
            1}        
            }
        );
        #ifndef VENGINE_NO_PROFILING
        TracyVkCollect(this->tracyContext[currentImageIndex], this->commandBuffers[currentImageIndex]);
        #endif 
            
    }
    /// Stop recording to a command buffer
    commandBuffers[currentImageIndex].end();
}


#ifndef VENGINE_NO_PROFILING 
void VulkanRenderer::getFrameThumbnailForTracy() //TODO: Update to use vma instead of regular deviceMemory...
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif   
    assert(false && "Please fix The usage of insertImageMemoryBarrier before trying to use this function...");
    vk::Image srcImage = this->swapChainImages[this->currentFrame].image;
    vk::ImageCreateInfo imageCreateInfo{};    
    imageCreateInfo.setImageType(vk::ImageType::e2D);
    imageCreateInfo.setFormat(vk::Format::eR8G8B8A8Unorm);      
    
    int width =0; 
    int height =0;

    SDL_GetWindowSize(this->window.sdl_window, &width, &height);
    imageCreateInfo.extent.setWidth(static_cast<uint32_t>(width));
    imageCreateInfo.extent.setHeight(static_cast<uint32_t>(height));
    imageCreateInfo.extent.setDepth(uint32_t(1));
    imageCreateInfo.setArrayLayers(uint32_t(1));
    imageCreateInfo.setMipLevels(uint32_t (1));
    imageCreateInfo.setInitialLayout(vk::ImageLayout::eUndefined);
    imageCreateInfo.setSamples(vk::SampleCountFlagBits::e1);
    imageCreateInfo.setTiling(vk::ImageTiling::eLinear);   /// Linear so we can read it (??)
    imageCreateInfo.setUsage(vk::ImageUsageFlagBits::eTransferDst);

    vk::Image dstImage;
    dstImage = this->mainDevice.logicalDevice.createImage(imageCreateInfo);

    vk::MemoryRequirements2 memoryRequirments;
    vk::MemoryAllocateInfo memoryAllocateInfo;
    vk::DeviceMemory dstImageMemory;
    memoryRequirments = this->mainDevice.logicalDevice.getImageMemoryRequirements2(dstImage);
    memoryAllocateInfo.setAllocationSize( memoryRequirments.memoryRequirements.size);

    memoryAllocateInfo.setMemoryTypeIndex( vengine_helper::findMemoryTypeIndex(
        this->mainDevice.physicalDevice, 
        memoryRequirments.memoryRequirements.memoryTypeBits, 
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent));
    
    dstImageMemory = this->mainDevice.logicalDevice.allocateMemory(memoryAllocateInfo);

    vk::BindImageMemoryInfoKHR bindImageMemoryInfo;
    bindImageMemoryInfo.setImage(dstImage);
    bindImageMemoryInfo.setMemory(dstImageMemory);
    bindImageMemoryInfo.setMemoryOffset(0);
    this->mainDevice.logicalDevice.bindImageMemory2(bindImageMemoryInfo);

    //VkCommandBuffer copyCmd = vengine_helper::beginCommandBuffer(this->mainDevice.logicalDevice, this->graphicsCommandPool);
    vk::CommandBuffer copyCmd = vengine_helper::beginCommandBuffer(this->mainDevice.logicalDevice, this->graphicsCommandPool);
    vengine_helper::insertImageMemoryBarrier(
        {
            .cmdBuffer     = copyCmd, 
            .image         = dstImage, 
            .srcAccessMask = vk::AccessFlagBits2::eNone,
            .dstAccessMask = vk::AccessFlagBits2::eTransferWrite, 
            .oldLayout     = vk::ImageLayout::eUndefined, 
            .newLayout     = vk::ImageLayout::eTransferDstOptimal
        }
        );
        
    vengine_helper::insertImageMemoryBarrier(
        {
            .cmdBuffer     = copyCmd, 
            .image         = srcImage, 
            .srcAccessMask = vk::AccessFlagBits2::eMemoryRead,
            .dstAccessMask = vk::AccessFlagBits2::eTransferRead, 
            .oldLayout     = vk::ImageLayout::ePresentSrcKHR, 
            .newLayout     = vk::ImageLayout::eTransferSrcOptimal
        }
        );

    vk::ImageCopy imageCopyRegion;
    imageCopyRegion.srcSubresource.setAspectMask(vk::ImageAspectFlagBits::eColor);
    imageCopyRegion.srcSubresource.setLayerCount(uint32_t (1));
    imageCopyRegion.dstSubresource.setAspectMask(vk::ImageAspectFlagBits::eColor);
    imageCopyRegion.dstSubresource.setLayerCount(uint32_t(1));
    imageCopyRegion.extent.setWidth(static_cast<uint32_t>(width));
    imageCopyRegion.extent.setHeight(static_cast<uint32_t>(height));
    imageCopyRegion.extent.setDepth(uint32_t (1));

    vengine_helper::insertImageMemoryBarrier(
        {
            .cmdBuffer     = copyCmd, 
            .image         = dstImage, 
            .srcAccessMask = vk::AccessFlagBits2::eTransferWrite,
            .dstAccessMask = vk::AccessFlagBits2::eMemoryRead, 
            .oldLayout     = vk::ImageLayout::eTransferDstOptimal,
            .newLayout     = vk::ImageLayout::eGeneral
        });


    vengine_helper::insertImageMemoryBarrier(
        {
            .cmdBuffer     = copyCmd, 
            .image         = srcImage, 
            .srcAccessMask = vk::AccessFlagBits2::eTransferRead, 
            .dstAccessMask = vk::AccessFlagBits2::eMemoryRead,
            .oldLayout     = vk::ImageLayout::eTransferSrcOptimal, 
            .newLayout     = vk::ImageLayout::ePresentSrcKHR
        });

    vengine_helper::endAndSubmitCommandBufferWithFences(
        this->mainDevice.logicalDevice, 
        this->graphicsCommandPool, 
        this->graphicsQueue, 
        copyCmd);
    
    vk::ImageSubresource subResource {vk::ImageAspectFlagBits::eColor, 0,0};
    vk::SubresourceLayout subResourceLayout{};

    subResourceLayout = this->mainDevice.logicalDevice.getImageSubresourceLayout(
        dstImage,
        subResource
    );
    
    const unsigned char* data = nullptr;

    data = (const unsigned char*)this->mainDevice.logicalDevice.mapMemory(
        dstImageMemory,
        0,
        VK_WHOLE_SIZE,
        vk::MemoryMapFlags()
    );
    
    if(false){ //NOLINT: TODO: this code is left for reference... might be removed when tracy recieves frame images properly...
        data += subResourceLayout.offset;

        int c = 0;    
        for(uint32_t i = 0; i < static_cast<uint32_t>(height); i++)
        {            
            auto *row = (unsigned int*)data;
            for(uint32_t j = 0; j < width;j++ )
            {                   
                /*
                (static_cast<char*>(this->tracyImage))[c++] = *(((char*)row + 2));
                (static_cast<char*>(this->tracyImage))[c++] = *(((char*)row + 1));
                (static_cast<char*>(this->tracyImage))[c++] = *(((char*)row) );        
                (static_cast<char*>(this->tracyImage))[c++] = *(((char*)row + 3));  
                */                               
                
                ((this->tracyImage))[c++] = *(((char*)row + 2));
                ((this->tracyImage))[c++] = *(((char*)row + 1));
                ((this->tracyImage))[c++] = *(((char*)row));        
                ((this->tracyImage))[c++] = *(((char*)row + 3));  
                
                /*
                ((this->tracyImage))[c++] = (((char*)row + 2));
                ((this->tracyImage))[c++] = (((char*)row + 1));
                ((this->tracyImage))[c++] = (((char*)row) ); 
                */       
                //((this->tracyImage))[c++] = (((char*)row + 3));  

                row++;
            }

            data += subResourceLayout.rowPitch;
        }
        //FrameImage(data,width,height,0,false);
        FrameImage(tracyImage,width,height,0,false);
        //FrameImage(tracyImage_temp,width,height,0,false);
    }else{
        FrameImage(data,width,height,0,false);
    }
    
    this->mainDevice.logicalDevice.unmapMemory(dstImageMemory);
    this->mainDevice.logicalDevice.freeMemory(dstImageMemory);
    this->mainDevice.logicalDevice.destroyImage(dstImage);

}
#endif
#ifndef VENGINE_NO_PROFILING 
void VulkanRenderer::allocateTracyImageMemory()
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    int width =0;
    int height =0;

    SDL_GetWindowSize(this->window.sdl_window, &width, &height);
    this->tracyImage = static_cast<char*>(CustomAlloc((static_cast<long>(height*width*4)) * sizeof(char)));
     
}
#endif
#ifndef VENGINE_NO_PROFILING 
void VulkanRenderer::initTracy()
{
    #ifndef VENGINE_NO_PROFILING
    /// Tracy stuff
    allocateTracyImageMemory();
    vk::DynamicLoader dl; 
    auto pfnvkGetPhysicalDeviceCalibrateableTimeDomainsEXT = dl.getProcAddress<PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT>("vkGetPhysicalDeviceCalibrateableTimeDomainsEXT");

    auto pfnvkGetCalibratedTimestampsEXT = dl.getProcAddress<PFN_vkGetCalibratedTimestampsEXT>("vkGetCalibratedTimestampsEXT");

    /// Create Tracy Vulkan Context
    this->tracyContext.resize(this->swapChainImages.size());
    for(size_t i = 0 ; i < this->swapChainImages.size(); i++){
        
        this->tracyContext[i] = TracyVkContextCalibrated(
            this->mainDevice.physicalDevice, 
            this->mainDevice.logicalDevice, 
            this->graphicsQueue, 
            commandBuffers[i],
            pfnvkGetPhysicalDeviceCalibrateableTimeDomainsEXT,
            pfnvkGetCalibratedTimestampsEXT
        );
        
        std::string name = "TracyVkContext_" + std::to_string(i);
        TracyVkContextName(this->tracyContext[i], name.c_str(), name.size());
    }
    TracyHelper::setVulkanRenderReference(this);
    TracyHelper::registerTracyParameterFunctions();
    
    #endif 
}
#endif

void VulkanRenderer::registerVkObjectDbgInfo(std::string name, vk::ObjectType type, uint64_t objectHandle)
{
#ifdef DEBUG

    vk::DebugUtilsObjectNameInfoEXT objInfo;
    objInfo.setPObjectName(name.c_str());
    objInfo.setObjectType(type);
    objInfo.setObjectHandle(objectHandle); //NOLINT: reinterpret cast is ok here...
    objInfo.setPNext(nullptr);    

    auto pfnSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(this->instance, "vkSetDebugUtilsObjectNameEXT"); //(!!)

    auto temp =  VkDebugUtilsObjectNameInfoEXT(objInfo);
    pfnSetDebugUtilsObjectNameEXT(this->mainDevice.logicalDevice, &temp); //(!!)

#endif    
}

void VulkanRenderer::initImgui()
{
    vk::DescriptorPoolSize pool_sizes[] =
	{
		{ vk::DescriptorType::eSampler, 1000 },
		{ vk::DescriptorType::eCombinedImageSampler, 1000 },
		{ vk::DescriptorType::eSampledImage, 1000 },
		{ vk::DescriptorType::eStorageImage, 1000 },
		{ vk::DescriptorType::eUniformTexelBuffer, 1000 },
		{ vk::DescriptorType::eStorageTexelBuffer, 1000 },
		{ vk::DescriptorType::eUniformBuffer, 1000 },
		{ vk::DescriptorType::eStorageBuffer, 1000 },
		{ vk::DescriptorType::eUniformBufferDynamic, 1000 },
		{ vk::DescriptorType::eStorageBufferDynamic, 1000 },
		{ vk::DescriptorType::eInputAttachment, 1000 }
	};

	vk::DescriptorPoolCreateInfo pool_info = {};	
	pool_info.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
	pool_info.maxSets = 1000;
	pool_info.poolSizeCount = std::size(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;

	this->descriptorPool_imgui = this->mainDevice.logicalDevice.createDescriptorPool(pool_info, nullptr);
	

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();    
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    
    io.ConfigWindowsResizeFromEdges = true;
    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
    

    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForVulkan(this->window.sdl_window);

    ImGui_ImplVulkan_InitInfo imguiInitInfo {};
    imguiInitInfo.Instance = this->instance;
    imguiInitInfo.PhysicalDevice = this->mainDevice.physicalDevice;
    imguiInitInfo.Device = this->mainDevice.logicalDevice;
    imguiInitInfo.QueueFamily = this->getQueueFamilies(this->mainDevice.physicalDevice).graphicsFamily;
    imguiInitInfo.Queue = this->graphicsQueue;
    //imguiInitInfo.PipelineCache = this->graphics_pipelineCache;
    imguiInitInfo.PipelineCache = VK_NULL_HANDLE;  //TODO: Imgui Pipeline Should have its own Cache! 
    imguiInitInfo.DescriptorPool = this->descriptorPool_imgui;
    imguiInitInfo.Subpass = 0; 
    imguiInitInfo.MinImageCount = 3; //TODO: Expose this information (available when createing the swapchain) and use it here
    imguiInitInfo.ImageCount = 3;    //TODO: Expose this information (available when createing the swapchain) and use it here
    imguiInitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT; //TODO: check correctness    
    imguiInitInfo.Allocator = nullptr;    //TODO: Can/should I pass in something VMA related here?
    //imguiInitInfo.CheckVkResultFn = ...? ;  //TODO: Check what tthis is? Callback to debug message log?
    ImGui_ImplVulkan_Init(&imguiInitInfo, this->renderPass_imgui);

    std::vector<vk::ImageView> attachment;    
    attachment.resize(1);

    commandPools_imgui.resize(this->swapChainImages.size());
    commandBuffers_imgui.resize(this->swapChainImages.size());
    frameBuffers_imgui.resize(this->swapChainImages.size());
    for(size_t i = 0; i < this->swapChainImages.size(); i++){
        createCommandPool(this->commandPools_imgui[i], vk::CommandPoolCreateFlagBits::eResetCommandBuffer, std::string("commandPools_imgui["+std::to_string(i)+"]"));
        createCommandBuffer(this->commandBuffers_imgui[i], this->commandPools_imgui[i], std::string("commandBuffers_imgui["+std::to_string(i)+"]"));        
    }
    this->createFramebuffer_imgui();

    this->mainDevice.logicalDevice.resetCommandPool(commandPools_imgui[0]);
    
    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    commandBuffers_imgui[0].begin(begin_info);
    

    ImGui_ImplVulkan_CreateFontsTexture(commandBuffers_imgui[0]);

    commandBuffers_imgui[0].end();
    
    vk::SubmitInfo end_info = {};        
    end_info.commandBufferCount = 1;
    end_info.pCommandBuffers = &commandBuffers_imgui[0];
        
    vk::Result result = this->graphicsQueue.submit(1, &end_info, VK_NULL_HANDLE);
    if(result != vk::Result::eSuccess){
        throw std::runtime_error("Failed to submit imgui fonts to graphics queue...");
    }

    this->mainDevice.logicalDevice.waitIdle();

    ImGui_ImplVulkan_DestroyFontUploadObjects();
    

}

void VulkanRenderer::createFramebuffer_imgui()
{
    std::vector<vk::ImageView> attachment;    
    attachment.resize(1);
    frameBuffers_imgui.resize(this->swapChainImages.size());
    for(size_t i = 0; i < this->swapChainImages.size(); i++){        
        attachment[0] = this->swapChainImages[i].imageView; //TODO: Check if this is rightt?...
        createFrameBuffer(this->frameBuffers_imgui[i], attachment, this->renderPass_imgui, this->swapChainExtent, std::string("frameBuffers_imgui["+std::to_string(i)+"]"));
    }
}

void VulkanRenderer::cleanupFramebuffer_imgui()
{
    for (auto framebuffer: this->frameBuffers_imgui) {
        this->mainDevice.logicalDevice.destroyFramebuffer(framebuffer);        
    }
}
