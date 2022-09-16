#include "VulkanRenderer.hpp"
#include "Utilities.hpp"
#include "assimp/Importer.hpp"
#include "defs.hpp"
#include "VulkanValidation.hpp"
#include "tracyHelper.hpp"
#include "Configurator.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <utility>
#include <fstream>
#include <set>
#include <vector>
#include <limits>               // Used to get the Max value of a uint32_t
#include <algorithm>            // Used for std::clamp...
#include "stb_image.h"
#include "VulkanDbg.hpp"
#include "Texture.hpp"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "backends/imgui_impl_vulkan.h"

#include "Input.hpp"
#include "Scene.hpp"
#include "MeshComponent.hpp"
#include "Log.hpp"

static void checkVkResult(VkResult err)
{
    if (err == 0)
        return;

    Log::error("Vulkan error from imgui: " + std::to_string(err));
}

using namespace vengine_helper::config;
int VulkanRenderer::init(Window* window, std::string&& windowName) {
    
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    
    this->window = window;

    try {
        this->instance.createInstance(*this->window);

        // Create the surface
        createSurface();

        // Pick a physical device based on certain requirements
        this->physicalDevice.pickPhysicalDevice(
            this->instance,
            this->surface,
            this->queueFamilies.getIndices()
        );

        // Create logical device
        this->device.createDevice(
            this->instance, 
            this->physicalDevice, 
            this->queueFamilies,

            // TODO: remove this
            this->dynamicDispatch
        );

        VulkanDbg::init(
            this->instance,
            this->device
        );

        VulkanDbg::registerVkObjectDbgInfo("Logical Device", vk::ObjectType::eDevice, reinterpret_cast<uint64_t>(vk::Device::CType(this->getVkDevice())));
        VulkanDbg::registerVkObjectDbgInfo("Graphics Queue", vk::ObjectType::eQueue, reinterpret_cast<uint64_t>(vk::Queue::CType(this->queueFamilies.getGraphicsQueue())));
        VulkanDbg::registerVkObjectDbgInfo("Presentation Queue", vk::ObjectType::eQueue, reinterpret_cast<uint64_t>(vk::Queue::CType(this->queueFamilies.getPresentQueue()))); // TODO: might be problematic.. since it can be same as Graphics Queue
             
        VulkanDbg::registerVkObjectDbgInfo("Surface",vk::ObjectType::eSurfaceKHR, reinterpret_cast<uint64_t>(vk::SurfaceKHR::CType(this->surface)));                
        VulkanDbg::registerVkObjectDbgInfo("PhysicalDevice",vk::ObjectType::ePhysicalDevice, reinterpret_cast<uint64_t>(vk::PhysicalDevice::CType(this->physicalDevice.getVkPhysicalDevice())));                
        VulkanDbg::registerVkObjectDbgInfo("Logical Device",vk::ObjectType::eDevice, reinterpret_cast<uint64_t>(vk::Device::CType(this->getVkDevice())));

        VmaAllocatorCreateInfo vmaAllocatorCreateInfo{};
        vmaAllocatorCreateInfo.device = this->getVkDevice();
        vmaAllocatorCreateInfo.physicalDevice = this->physicalDevice.getVkPhysicalDevice();
        vmaAllocatorCreateInfo.instance = this->instance.getVkInstance();
        vmaAllocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
        if(vmaCreateAllocator(&vmaAllocatorCreateInfo, &this->vma) != VK_SUCCESS)
        {
            Log::error("Could not create the VMA (vulkan memory allocator)!");
        }

        this->setupDebugMessenger();  // Used when we use Validation Layers to trigger errors/warnings/etc.

        this->swapchain.createSwapchain(
            *this->window,
            this->physicalDevice,
            this->device,
            this->surface,
            this->queueFamilies,
            this->vma
        );
        
        this->createRenderPass_Base();
        this->createRenderPass_Imgui();
        this->createDescriptorSetLayout();
        
        this->createPushConstantRange();

        this->createGraphicsPipeline_Base();
        
        this->swapchain.createFramebuffers(this->renderPass_base);
        this->createCommandPool();
        
        this->createCommandBuffers();

        this->createTextureSampler();
        
        this->createUniformBuffers();

        this->createDescriptorPool();

        this->allocateDescriptorSets();
        this->createDescriptorSets();
        
        this->createInputDescriptorSets();

        this->createSynchronisation();        

        this->updateUBO_camera_Projection(); //TODO: Allow for more cameras! 
        this->updateUBO_camera_view(
            glm::vec3(DEF<float>(CAM_EYE_X),DEF<float>(CAM_EYE_Y),DEF<float>(CAM_EYE_Z)),
            glm::vec3(DEF<float>(CAM_TARGET_X),DEF<float>(CAM_TARGET_Y), DEF<float>(CAM_TARGET_Z)));

        // Setup Fallback Texture: Let first Texture be default if no other texture is found.
        this->createTexture("missing_texture.png");

#ifndef VENGINE_NO_PROFILING
        this->initTracy();
#endif
        this->initImgui();
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
    this->modelList[modelIndex].setModelMatrix(newModel);
}

void VulkanRenderer::generateVmaDump()
{
    char* vma_dump;
    vmaBuildStatsString(this->vma,&vma_dump,VK_TRUE);
    std::ofstream file("vma_dump.json");
    file << vma_dump << std::endl;
    file.close();
    
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
    this->device.waitIdle(); // Dont destroy semaphores before they are done
    
    ImGui_ImplVulkan_Shutdown();
    this->window->shutdownImgui();
    ImGui::DestroyContext();

    this->cleanupFramebuffer_imgui();

    this->getVkDevice().destroyRenderPass(this->renderPass_imgui);
    this->getVkDevice().destroyDescriptorPool(this->descriptorPool_imgui);
    
#ifndef VENGINE_NO_PROFILING
    CustomFree(this->tracyImage);

    for(auto &tracy_context : this->tracyContext)
    {
        TracyVkDestroy(tracy_context);
    }
#endif

    for(auto & i : modelList)
    {
        i.destroyMeshModel();
    }

    this->getVkDevice().destroyDescriptorPool(this->inputDescriptorPool);
    this->getVkDevice().destroyDescriptorSetLayout(this->inputSetLayout);

    this->getVkDevice().destroyDescriptorPool(this->samplerDescriptorPool);
    this->getVkDevice().destroyDescriptorSetLayout(this->samplerDescriptorSetLayout);

    this->getVkDevice().destroySampler(this->textureSampler);

    for(size_t i = 0; i < this->textureImages.size();i++)
    {
        this->getVkDevice().destroyImageView(this->textureImageViews[i]);
        this->getVkDevice().destroyImage(this->textureImages[i]);
        vmaFreeMemory(this->vma,this->textureImageMemory[i]);
    }

    this->getVkDevice().destroyDescriptorPool(this->descriptorPool);
    this->getVkDevice().destroyDescriptorSetLayout(this->descriptorSetLayout);

    for(size_t i = 0; i < this->viewProjection_uniformBuffer.size(); i++)
    {
        this->getVkDevice().destroyBuffer(this->viewProjection_uniformBuffer[i]);
        vmaFreeMemory(this->vma, this->viewProjection_uniformBufferMemory[i]);
    }

    for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        this->getVkDevice().destroySemaphore(this->renderFinished[i]);
        this->getVkDevice().destroySemaphore(this->imageAvailable[i]);
        this->getVkDevice().destroyFence(this->drawFences[i]);        
    }

    this->getVkDevice().destroyCommandPool(this->graphicsCommandPool);
    
    this->getVkDevice().destroyPipelineCache(this->graphics_pipelineCache);
    this->getVkDevice().destroyPipeline(this->secondGraphicsPipeline);
    this->getVkDevice().destroyPipelineLayout(this->secondPipelineLayout);

    this->getVkDevice().destroyPipeline(this->graphicsPipeline);
    this->getVkDevice().destroyPipelineLayout(this->pipelineLayout);
    this->getVkDevice().destroyRenderPass(this->renderPass_base);

    this->swapchain.cleanup();

    this->instance.destroy(this->surface); //NOTE: No warnings/errors if we run this line... Is it useless? Mayber gets destroyed by SDL?
    
    this->generateVmaDump();
    vmaDestroyAllocator(this->vma);

    this->device.cleanup();

    if (isValidationLayersEnabled())
    {
        this->instance.getVkInstance().destroyDebugUtilsMessengerEXT(
            this->debugMessenger,
            nullptr,
            this->dynamicDispatch
        );
    }
    this->instance.cleanup();
}

void VulkanRenderer::draw(Scene* scene)
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
        
        //TODO: PROFILING; Check if its faster to have wait for fences after acquire image or not...
        // Wait for The Fence to be signaled from last Draw for this currrent Frame; 
        // This will freeze the CPU operations here and wait for the Fence to open
        vk::Bool32 waitForAllFences = VK_TRUE;

        auto result = this->getVkDevice().waitForFences(
            uint32_t(1),                        // number of Fences to wait on
            &this->drawFences[this->currentFrame],    // Which Fences to wait on
            waitForAllFences,                   // should we wait for all Fences or not?              
            std::numeric_limits<uint64_t>::max());
        if(result != vk::Result::eSuccess) 
        {
            Log::error("Failed to wait for all fences!");
        }
    }

    // Get scene camera and update view matrix
    Camera* camera = scene->getMainCamera();
    bool deleteCamera = false;
    if (camera)
    {
        Transform& transform = scene->getComponent<Transform>(scene->getMainCameraID());
        camera->view = glm::lookAt(
            transform.position,
            transform.position + transform.forward(),
            transform.up()
        );
    }
    else
    {
        Log::error("No main camera exists!");
        camera = new Camera((float)this->swapchain.getWidth() / (float)this->swapchain.getHeight());
        camera->view = uboViewProjection.view;
        deleteCamera = true;
    }

    unsigned int imageIndex = 0 ;
    {
        #ifndef VENGINE_NO_PROFILING
        ZoneNamedN(draw_zone2, "Get Next Image", true); //:NOLINT   
        #endif 
        // -- Get Next Image -- 
        //1. Get Next available image image to draw to and set a Semaphore to signal when we're finished with the image 

        vk::Result result{};
        // Retrieve the Index of the image to be displayed.
        std::tie(result, imageIndex) = this->getVkDevice().acquireNextImageKHR( 
            this->swapchain.getVkSwapchain(),
            std::numeric_limits<uint64_t>::max(),   // How long to wait before the Image is retrieved, crash if reached. 
                                                    // We dont want to use a timeout, so we make it as big as possible.
            this->imageAvailable[this->currentFrame],     // The Semaphore to signal, when it's available to be used!
            VK_NULL_HANDLE                          // The Fence to signal, when it's available to be used...(??)
        );
        if(result == vk::Result::eErrorOutOfDateKHR){
            this->recreateSwapchain(camera);    
            return;
        }
        else if(result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) 
        {
            Log::error("Failed to AcquireNextImage!");
        }

        // Close the Fence behind us if work is being submitted...
        result = this->getVkDevice().resetFences(
            uint32_t(1),
            &this->drawFences[this->currentFrame]);
        if(result != vk::Result::eSuccess) 
        {
            Log::error("Failed to reset fences!");
        }
    }
    
    // Set view and projection in ubo
    uboViewProjection.view = camera->view;
    uboViewProjection.projection = camera->projection;
    if (deleteCamera) { delete camera; }

    // Update the Uniform Buffers
    this->updateUniformBuffers();

    // ReRecord the current CommandBuffer! In order to update any Push Constants
    recordRenderPassCommands_Base(scene, imageIndex);
    //recordDynamicRenderingCommands(imageIndex); 
    
    // Submit to graphics queue
    {
        #ifndef VENGINE_NO_PROFILING
        ZoneNamedN(draw_zone3, "Wait for Semaphore", true); //:NOLINT   
        #endif 

        // -- Submit command buffer to Render -- 
        //2. Submit command buffer to queue for execution, making sure it waits for the image to be signalled as 
        //   available before drawing and signals when it has finished renedering. 
        
        std::array<vk::PipelineStageFlags2, 1> waitStages = {               // Definies What stages the Semaphore have to wait on.        
            vk::PipelineStageFlagBits2::eColorAttachmentOutput  // Stage: Start drawing to the Framebuffer...
        };
        
        vk::SemaphoreSubmitInfo wait_semaphoreSubmitInfo;
        wait_semaphoreSubmitInfo.setSemaphore(this->imageAvailable[this->currentFrame]);
        wait_semaphoreSubmitInfo.setStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput); // (!!)
        //wait_semaphoreSubmitInfo.setStageMask(vk::PipelineStageFlagBits2::eAllCommands); // (!!)(??)
        wait_semaphoreSubmitInfo.setDeviceIndex(uint32_t(1));                            // 0: sets all devices in group 1 to valid... bad or good?

        vk::SemaphoreSubmitInfo signal_semaphoreSubmitInfo;
        signal_semaphoreSubmitInfo.setSemaphore(this->renderFinished[this->currentFrame]);
        signal_semaphoreSubmitInfo.setStageMask(vk::PipelineStageFlags2());      // Stages to check semaphores at    

        std::vector<vk::CommandBufferSubmitInfo> commandBufferSubmitInfos{
            vk::CommandBufferSubmitInfo{this->commandBuffers[this->currentFrame]}
        };        
        
        vk::SubmitInfo2 submitInfo {};      
        submitInfo.setWaitSemaphoreInfoCount(uint32_t(1));
        // !!!submitInfo.setWaitSemaphoreInfos(const vk::ArrayProxyNoTemporaries<const vk::SemaphoreSubmitInfo> &waitSemaphoreInfos_)
        submitInfo.setPWaitSemaphoreInfos(&wait_semaphoreSubmitInfo); // Pointer to the semaphore to wait on.
        submitInfo.setCommandBufferInfoCount(commandBufferSubmitInfos.size()); 
        submitInfo.setPCommandBufferInfos(commandBufferSubmitInfos.data()); // Pointer to the CommandBuffer to execute
        submitInfo.setSignalSemaphoreInfoCount(uint32_t(1));
        submitInfo.setPSignalSemaphoreInfos(&signal_semaphoreSubmitInfo);// Semaphore that will be signaled when 
        
        // Submit The CommandBuffers to the Queue to begin drawing to the framebuffers
        this->queueFamilies.getGraphicsQueue().submit2(
            submitInfo, 
            this->drawFences[this->currentFrame]
        ); // drawing, signal this Fence to open!
    }

    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();        
        ImGui::RenderPlatformWindowsDefault();
    }

    // Submit to presentation queue
    {
        #ifndef VENGINE_NO_PROFILING
        ZoneNamedN(draw_zone4, "Present Image", true); //:NOLINT   
        #endif 
        // -- Present Rendered Image to Screen -- 
        //3. Present image to screen when it has signalled finished rendering.
        vk::PresentInfoKHR presentInfo{};
        presentInfo.setWaitSemaphoreCount(uint32_t (1));
        presentInfo.setPWaitSemaphores(&this->renderFinished[this->currentFrame]);  // Semaphore to Wait on before Presenting
        presentInfo.setSwapchainCount(uint32_t (1));    
        presentInfo.setPSwapchains(&this->swapchain.getVkSwapchain());                         // Swapchain to present the image to
        presentInfo.setPImageIndices(&imageIndex);                            // Index of images in swapchains to present                

        // Submit the image to the presentation Queue
        vk::Result resultvk = 
            this->queueFamilies.getPresentQueue().presentKHR(&presentInfo);
        if (resultvk == vk::Result::eErrorOutOfDateKHR || resultvk == vk::Result::eSuboptimalKHR || this->windowResized )
        {
            this->windowResized = false;       
            this->recreateSwapchain(camera);
        }
        else if(resultvk != vk::Result::eSuccess) 
        {
            Log::error("Failed to present Image!");
        }
    }

    // Update current Frame for next draw!
    this->currentFrame = (this->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    
#ifndef VENGINE_NO_PROFILING    
    FrameMarkEnd(draw_frame);
#endif        
}

void VulkanRenderer::initMeshes(Scene* scene)
{
    auto tView = scene->getSceneReg().view<MeshComponent>();
    tView.each([this](MeshComponent& meshComponent)
    {
        meshComponent.meshID = this->createModel("ghost.obj");
    });
}

void VulkanRenderer::setupDebugMessenger() 
{
    if(!isValidationLayersEnabled())
    { return; }

    // In this function we define what sort of message we want to receive...
    vk::DebugUtilsMessengerCreateInfoEXT createInfo{};
    VulkanDbg::populateDebugMessengerCreateInfo(createInfo);
    
    auto result = 
        this->instance.getVkInstance().createDebugUtilsMessengerEXT(
            &createInfo, 
            nullptr, 
            &this->debugMessenger, 
            this->dynamicDispatch);
    if (result != vk::Result::eSuccess) 
    {
        Log::error("Failed to create Debug Messenger!");
    }
}

void VulkanRenderer::createSurface() 
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    //Using SDL to create WindowSurface, to make it cross platform
    //Creates a surface create info struct configured for how SDL handles windows.    
    this->window->createVulkanSurface(this->instance, this->surface);
}

void VulkanRenderer::recreateSwapchain(Camera* camera)
{
    this->device.waitIdle();
    
    this->getVkDevice().freeDescriptorSets(
        this->inputDescriptorPool, 
        this->inputDescriptorSets
    );
    cleanupFramebuffer_imgui();

    this->swapchain.recreateSwapchain(this->renderPass_base);
    createFramebuffer_imgui();

    this->createDescriptorSets();
    this->createInputDescriptorSets();

    ImGui_ImplVulkan_SetMinImageCount(this->swapchain.getNumMinimumImages());

    // Take new aspect ratio into account for the camera
    camera->aspectRatio = (float) this->swapchain.getWidth() / (float)swapchain.getHeight();
    camera->projection = glm::perspective(camera->fov, camera->aspectRatio, 0.1f, 100.0f);
    camera->invProjection = glm::inverse(camera->projection);
}

void VulkanRenderer::cleanupRenderBass_Imgui()
{
    this->getVkDevice().destroyRenderPass(this->renderPass_imgui);
}

void VulkanRenderer::cleanupRenderBass_Base()
{
    this->getVkDevice().destroyRenderPass(this->renderPass_base);
}

void VulkanRenderer::createGraphicsPipeline_Base() 
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    // read in SPIR-V code of shaders
    auto vertexShaderCode = vengine_helper::readShaderFile("shader.vert.spv");
    auto fragShaderCode = vengine_helper::readShaderFile("shader.frag.spv");

    // Build Shader Modules to link to Graphics Pipeline
    // Create Shader Modules
    vk::ShaderModule vertexShaderModule = this->createShaderModule(vertexShaderCode);
    vk::ShaderModule fragmentShaderModule = this->createShaderModule(fragShaderCode);
    VulkanDbg::registerVkObjectDbgInfo("ShaderModule VertexShader", vk::ObjectType::eShaderModule, reinterpret_cast<uint64_t>(vk::ShaderModule::CType(vertexShaderModule)));
    VulkanDbg::registerVkObjectDbgInfo("ShaderModule fragmentShader", vk::ObjectType::eShaderModule, reinterpret_cast<uint64_t>(vk::ShaderModule::CType(fragmentShaderModule)));

    // --- SHADER STAGE CREATION INFORMATION ---
    // Vertex Stage Creation Information
    vk::PipelineShaderStageCreateInfo vertexShaderStageCreateInfo {};
    vertexShaderStageCreateInfo.setStage(vk::ShaderStageFlagBits::eVertex);                             // Shader stage name
    vertexShaderStageCreateInfo.setModule(vertexShaderModule);                                    // Shader Modual to be used by Stage
    vertexShaderStageCreateInfo.setPName("main");                                                 //Name of the vertex Shaders main function (function to run)

    // Fragment Stage Creation Information
    vk::PipelineShaderStageCreateInfo fragmentShaderPipelineCreatInfo {};
    fragmentShaderPipelineCreatInfo.setStage(vk::ShaderStageFlagBits::eFragment);                       // Shader Stage Name
    fragmentShaderPipelineCreatInfo.setModule(fragmentShaderModule);                              // Shader Module used by stage
    fragmentShaderPipelineCreatInfo.setPName("main");                                             // name of the fragment shader main function (function to run)
    
    // Put shader stage creation infos into array
    // graphics pipeline creation info requires array of shader stage creates
    std::array<vk::PipelineShaderStageCreateInfo, 2> pipelineShaderStageCreateInfos {
            vertexShaderStageCreateInfo,
            fragmentShaderPipelineCreatInfo
    };

    // -- FIXED SHADER STAGE CONFIGURATIONS -- 

    // How the data for a single vertex (including info such as position, colour, texture coords, normals, etc)
    // is as a whole.
    vk::VertexInputBindingDescription bindingDescription;
    bindingDescription.setBinding(uint32_t (0));                 // Can bind multiple streams of data, this defines which one.
    bindingDescription.setStride(sizeof(Vertex));    // Size of a single Vertex Object
    bindingDescription.setInputRate(vk::VertexInputRate::eVertex); // How to move between data after each vertex...
    
    // How the Data  for an attribute is definied within a vertex    
    std::array<vk::VertexInputAttributeDescription, 3> attributeDescriptions{}; 

    // Position Attribute: 
    attributeDescriptions[0].setBinding(uint32_t (0));                           // which binding the data is at (should be same as above)
    attributeDescriptions[0].setLocation(uint32_t (0));                           // Which Location in shader where data will be read from
    attributeDescriptions[0].setFormat(vk::Format::eR32G32B32Sfloat);  // Format the data will take (also helps define size of data)
    attributeDescriptions[0].setOffset(offsetof(Vertex, pos));       // Sets the offset of our struct member Pos (where this attribute is defined for a single vertex...)

    // Color Attribute.
    attributeDescriptions[1].setBinding(uint32_t (0));                         
    attributeDescriptions[1].setLocation(uint32_t (1));                         
    attributeDescriptions[1].setFormat(vk::Format::eR32G32B32Sfloat);
    attributeDescriptions[1].setOffset(offsetof(Vertex, col));

    // Texture Coorinate Attribute (uv): 
    attributeDescriptions[2].setBinding(uint32_t (0));
    attributeDescriptions[2].setLocation(uint32_t (2));
    attributeDescriptions[2].setFormat(vk::Format::eR32G32Sfloat);      // Note; only RG, since it's a 2D image we don't use the depth and thus we only need RG and not RGB
    attributeDescriptions[2].setOffset(offsetof(Vertex, tex));


    // -- VERTEX INPUT --
    vk::PipelineVertexInputStateCreateInfo vertexInputCreateInfo;
    vertexInputCreateInfo.setVertexBindingDescriptionCount(uint32_t (1));
    vertexInputCreateInfo.setPVertexBindingDescriptions(&bindingDescription);
    vertexInputCreateInfo.setVertexAttributeDescriptionCount(static_cast<uint32_t>(attributeDescriptions.size()));
    vertexInputCreateInfo.setPVertexAttributeDescriptions(attributeDescriptions.data());      

    // -- INPUT ASSEMBLY --
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo;
    inputAssemblyStateCreateInfo.setTopology(vk::PrimitiveTopology::eTriangleList);        // Primitive type to assemble primitives from ;
    inputAssemblyStateCreateInfo.setPrimitiveRestartEnable(VK_FALSE);                     // Allow overrideing tof "strip" topology to start new primitives

    // -- VIEWPORT & SCISSOR ---
    // Create viewport info struct
    vk::Viewport viewport;    
    viewport.setX(0.0F);        // x start coordinate
    viewport.setY(0.0F);        // y start coordinate
    viewport.setWidth(static_cast<float>(this->swapchain.getWidth()));       // width of viewport
    viewport.setHeight(static_cast<float>(this->swapchain.getHeight()));     // height of viewport
    viewport.setMinDepth(0.0F);     // min framebuffer depth
    viewport.setMaxDepth(1.0F);     // max framebuffer depth
    //create a scissor info struct
    vk::Rect2D scissor;
    scissor.offset = vk::Offset2D{0, 0};                            // Offset to use Region from
    scissor.extent = this->swapchain.getVkExtent();                   // Extent to sdescribe region to use, starting at offset

    vk::PipelineViewportStateCreateInfo viewportStateCreateInfo = {};
    viewportStateCreateInfo.setViewportCount(uint32_t(1));
    viewportStateCreateInfo.setPViewports(&viewport);
    viewportStateCreateInfo.setScissorCount(uint32_t(1));
    viewportStateCreateInfo.setPScissors(&scissor);


    // --- DYNAMIC STATES ---
    // dynamic states to enable... NOTE: to change some stuff you might need to reset it first... example viewport...
    std::vector<vk::DynamicState > dynamicStateEnables;
    dynamicStateEnables.push_back(vk::DynamicState::eViewport); // Dynamic Viewport    : can resize in command buffer with vkCmdSetViewport(command, 0,1,&viewport)
    dynamicStateEnables.push_back(vk::DynamicState::eScissor);  // Dynamic Scissor     : Can resize in commandbuffer with vkCmdSetScissor(command, 0,1, &viewport)

    // Dynamic state creation info
    vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};    
    dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
    dynamicStateCreateInfo.pDynamicStates = dynamicStateEnables.data();

    // --- RASTERIZER ---
    vk::PipelineRasterizationStateCreateInfo rasterizationStateCreateInfo;
    rasterizationStateCreateInfo.setDepthClampEnable(VK_FALSE);         // Change if fragments beyond near/far planes are clipped / clamped  (Requires depthclamp as a device features...)
    rasterizationStateCreateInfo.setRasterizerDiscardEnable(VK_FALSE);// Wether to discard data or skip rasterizzer. Never creates fragments, only suitable for pipleine without framebuffer output
    rasterizationStateCreateInfo.setPolygonMode(vk::PolygonMode::eFill);// How to handle filling point betweeen vertices...
    rasterizationStateCreateInfo.setLineWidth(1.F);                  // how thiock lines should be when drawn (other than 1 requires device features...)
    rasterizationStateCreateInfo.setCullMode(vk::CullModeFlagBits::eBack);  // Which face of tri to cull
    rasterizationStateCreateInfo.setFrontFace(vk::FrontFace::eCounterClockwise);// Since our Projection matrix now has a inverted Y-axis (GLM is right handed, but vulkan is left handed)
                                                                    // winding order to determine which side is front
    rasterizationStateCreateInfo.setDepthBiasEnable(VK_FALSE);        // Wether to add depthbiaoas to fragments (to remove shadowacne...)

    // --- MULTISAMPLING ---
    vk::PipelineMultisampleStateCreateInfo multisampleStateCreateInfo;
    multisampleStateCreateInfo.setSampleShadingEnable(VK_FALSE);                      // Enable multisample shading or not
    multisampleStateCreateInfo.setRasterizationSamples(vk::SampleCountFlagBits::e1);        // number of samples to use per fragment


    // --- BLENDING --
    
    // Blend attachment State (how blending is handled)
    vk::PipelineColorBlendAttachmentState  colorState;
    colorState.setColorWriteMask(vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eB // Colours to apply blending to
                | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eR);
    colorState.setBlendEnable(VK_TRUE);                                                       // enable blending

    // Blending uses Equation: (srcColorBlendFactor * new Colour) colorBlendOp (dstColorBlendFactor * old Colour)
    colorState.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha);
    colorState.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha);
    colorState.setColorBlendOp(vk::BlendOp::eAdd);

    colorState.setSrcAlphaBlendFactor(vk::BlendFactor::eOne);
    colorState.setDstAlphaBlendFactor(vk::BlendFactor::eZero);
    colorState.setAlphaBlendOp(vk::BlendOp::eAdd);

    vk::PipelineColorBlendStateCreateInfo colorBlendingCreateInfo;
    colorBlendingCreateInfo.setLogicOpEnable(VK_FALSE);               // Alternative to calculations is to use logical Operations
    colorBlendingCreateInfo.setAttachmentCount(uint32_t (1));
    colorBlendingCreateInfo.setPAttachments(&colorState);

    // --- PIPELINE LAYOUT ---

    // We have two Descriptor Set Layouts, 
    // One for View and Projection matrices Uniform Buffer, and the other one for the texture sampler!
    std::array<vk::DescriptorSetLayout, 2> descriptorSetLayouts{
        this->descriptorSetLayout,
        this->samplerDescriptorSetLayout
    };

    vk::PipelineLayoutCreateInfo  pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.setSetLayoutCount(static_cast<uint32_t>(descriptorSetLayouts.size()));
    pipelineLayoutCreateInfo.setPSetLayouts(descriptorSetLayouts.data());
    pipelineLayoutCreateInfo.setPushConstantRangeCount(uint32_t(1));                    // We only have One Push Constant range we want to use
    pipelineLayoutCreateInfo.setPPushConstantRanges(&this->pushConstantRange);// the Push Constant Range we want to use

    // --- DEPTH STENCIL TESING ---
    vk::PipelineDepthStencilStateCreateInfo depthStencilCreateInfo{};
    depthStencilCreateInfo.setDepthTestEnable(VK_TRUE);              // Enable Depth Testing; Check the Depth to determine if it should write to the fragment
    depthStencilCreateInfo.setDepthWriteEnable(VK_TRUE);              // enable writing to Depth Buffer; To make sure it replaces old values
    depthStencilCreateInfo.setDepthCompareOp(vk::CompareOp::eLess);   // Describes that we want to replace the old values IF new values are smaller/lesser.
    depthStencilCreateInfo.setDepthBoundsTestEnable(VK_FALSE);             // In case we want to use as Min and a Max Depth; if depth Values exist between two bounds... 
    depthStencilCreateInfo.setStencilTestEnable(VK_FALSE);             // Whether to enable the Stencil Test; we dont use it so we let it be disabled

    this->pipelineLayout = this->getVkDevice().createPipelineLayout(pipelineLayoutCreateInfo);
    VulkanDbg::registerVkObjectDbgInfo("VkPipelineLayout GraphicsPipelineLayout", vk::ObjectType::ePipelineLayout, reinterpret_cast<uint64_t>(vk::PipelineLayout::CType(this->pipelineLayout)));
    // --- RENDER PASS ---
    //createRenderPass(); <-- This is done in the init-function!

    // -- GRAPHICS PIPELINE CREATION --
    vk::GraphicsPipelineCreateInfo pipelineCreateInfo;
    pipelineCreateInfo.setStageCount(uint32_t(2));                                          // Number of shader stages
    pipelineCreateInfo.setPStages(pipelineShaderStageCreateInfos.data());         // List of Shader stages
    pipelineCreateInfo.setPVertexInputState(&vertexInputCreateInfo);              // All the fixed function Pipeline states
    pipelineCreateInfo.setPInputAssemblyState(&inputAssemblyStateCreateInfo);
    pipelineCreateInfo.setPViewportState(&viewportStateCreateInfo);
    pipelineCreateInfo.setPDynamicState(&dynamicStateCreateInfo);
    pipelineCreateInfo.setPRasterizationState(&rasterizationStateCreateInfo);
    pipelineCreateInfo.setPMultisampleState(&multisampleStateCreateInfo);
    pipelineCreateInfo.setPColorBlendState(&colorBlendingCreateInfo);
    pipelineCreateInfo.setPDepthStencilState(&depthStencilCreateInfo);
    pipelineCreateInfo.setLayout(this->pipelineLayout);                                 // Pipeline layout pipeline should use
    pipelineCreateInfo.setRenderPass(this->renderPass_base);                                 // Render pass description the pipeline is compatible with
    pipelineCreateInfo.setSubpass(uint32_t(0));                                             // subpass of render pass to use with pipeline

    // Pipeline Derivatives : Can Create multiple pipelines that derive from one another for optimization
    pipelineCreateInfo.setBasePipelineHandle(nullptr); // Existing pipeline to derive from ...
    pipelineCreateInfo.setBasePipelineIndex(int32_t(-1));              // or index of pipeline being created to derive from ( in case of creating multiple of at once )

    // Create Graphics Pipeline
    vk::Result result{};
    std::tie(result , this->graphicsPipeline) = getVkDevice().createGraphicsPipeline(nullptr,pipelineCreateInfo);
    if(result != vk::Result::eSuccess)
    {
        Log::error("Could not create Pipeline");
    }
    VulkanDbg::registerVkObjectDbgInfo("VkPipeline GraphicsPipeline", vk::ObjectType::ePipeline, reinterpret_cast<uint64_t>(vk::Pipeline::CType(this->graphicsPipeline)));

    //Destroy Shader Moduels, no longer needed after pipeline created
    this->getVkDevice().destroyShaderModule(vertexShaderModule);
    this->getVkDevice().destroyShaderModule(fragmentShaderModule);


    //////////////////////
    //////////////////////
    //////////////////////
    //////////////////////
    
    // - CREATE SECOND PASS PIPELINE - 
    // second Pass Shaders
    auto secondVertexShaderCode     = vengine_helper::readShaderFile("second.vert.spv");
    auto secondFragmentShaderCode     = vengine_helper::readShaderFile("second.frag.spv");    

    // Build Shaders
    vk::ShaderModule secondVertexShaderModule = createShaderModule(secondVertexShaderCode);
    vk::ShaderModule secondFragmentShaderModule = createShaderModule(secondFragmentShaderCode);
    VulkanDbg::registerVkObjectDbgInfo("ShaderModule Second_VertexShader", vk::ObjectType::eShaderModule, reinterpret_cast<uint64_t>(vk::ShaderModule::CType(secondVertexShaderModule)));
    VulkanDbg::registerVkObjectDbgInfo("ShaderModule Second_fragmentShader", vk::ObjectType::eShaderModule, reinterpret_cast<uint64_t>(vk::ShaderModule::CType(secondFragmentShaderModule)));

    // --- SHADER STAGE CREATION INFORMATION ---
    // Vertex Stage Creation Information
    vertexShaderStageCreateInfo.module = secondVertexShaderModule;                              // Shader Modual to be used by Stage

    // Fragment Stage Creation Information    
    fragmentShaderPipelineCreatInfo.module = secondFragmentShaderModule;                        // Shader Module used by stage    

    // Put shader stage creation infos into array
    // graphics pipeline creation info requires array of shader stage creates
    std::array<vk::PipelineShaderStageCreateInfo,2> secondPipelineShaderStageCreateInfos = {
            vertexShaderStageCreateInfo,
            fragmentShaderPipelineCreatInfo
    };
    
     // -- VERTEX INPUT --
    vertexInputCreateInfo.setVertexBindingDescriptionCount(uint32_t(0));
    vertexInputCreateInfo.setPVertexBindingDescriptions(nullptr);
    vertexInputCreateInfo.setVertexAttributeDescriptionCount(uint32_t (0));
    vertexInputCreateInfo.setPVertexAttributeDescriptions(nullptr);  

    // --- DEPTH STENCIL TESING ---
    depthStencilCreateInfo.setDepthWriteEnable(VK_FALSE);             // DISABLE writing to Depth Buffer; To make sure it replaces old values

    // Create New pipeline Layout
    vk::PipelineLayoutCreateInfo secondPipelineLayoutCreateInfo{};
    secondPipelineLayoutCreateInfo.setSetLayoutCount(uint32_t (1));
    secondPipelineLayoutCreateInfo.setPSetLayouts(&this->inputSetLayout);
    secondPipelineLayoutCreateInfo.setPushConstantRangeCount(uint32_t (0));
    secondPipelineLayoutCreateInfo.setPPushConstantRanges(nullptr);
    // createPipelineLayout
    this->secondPipelineLayout = this->getVkDevice().createPipelineLayout(secondPipelineLayoutCreateInfo);
    VulkanDbg::registerVkObjectDbgInfo("VkPipelineLayout Second_GraphicsPipelineLayout", vk::ObjectType::ePipelineLayout, reinterpret_cast<uint64_t>(vk::PipelineLayout::CType(this->secondPipelineLayout)));
    
     // -- GRAPHICS PIPELINE CREATION --
    pipelineCreateInfo.setPStages(secondPipelineShaderStageCreateInfos.data());                // List of Shader stages
    pipelineCreateInfo.setLayout(this->secondPipelineLayout);                      // Pipeline layout pipeline should use
    pipelineCreateInfo.setSubpass(uint32_t (1));                                             // Use Subpass 2 (index 1...)

    // Create Graphics Pipeline
    std::tie(result, this->secondGraphicsPipeline) = this->getVkDevice().createGraphicsPipeline(nullptr, pipelineCreateInfo);
    if(result != vk::Result::eSuccess)
    {
        Log::error("Could not create Second Pipeline");
    }
    VulkanDbg::registerVkObjectDbgInfo("VkPipeline Second_GraphicsPipeline", vk::ObjectType::ePipeline, reinterpret_cast<uint64_t>(vk::Pipeline::CType(this->secondGraphicsPipeline)));

    //Destroy Second Shader Moduels, no longer needed after pipeline created
    this->getVkDevice().destroyShaderModule(secondVertexShaderModule);
    this->getVkDevice().destroyShaderModule(secondFragmentShaderModule);

}

void VulkanRenderer::createGraphicsPipeline_DynamicRendering() //NOLINT: 
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    static const char* pipeline_cache_name = "pipeline_cache.data";
        // Pipeline Cache

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
        if(this->getVkDevice().createPipelineCache(&pipelineCacheCreateInfo, nullptr, &graphics_pipelineCache) != vk::Result::eSuccess)
        {
            Log::error("Failed to create PipelineCache");
        }
	}
    catch(std::exception &e)
    {
        std::cout << "Could not load "<<pipeline_cache_name<<" from disk...\n";

        vk::PipelineCacheCreateInfo pipelineCacheCreateInfo{};
        pipelineCacheCreateInfo.setInitialDataSize(0);
        pipelineCacheCreateInfo.setPInitialData(nullptr);
        if(this->getVkDevice().createPipelineCache(&pipelineCacheCreateInfo, nullptr, &graphics_pipelineCache) != vk::Result::eSuccess)
        {
            Log::error("Failed to create PipelineCache");
        }
    }

	{
    // read in SPIR-V code of shaders
    auto vertexShaderCode = vengine_helper::readShaderFile("shader.vert.spv");
    auto fragShaderCode = vengine_helper::readShaderFile("shader.frag.spv");

    // Build Shader Modules to link to Graphics Pipeline
    // Create Shader Modules
    vk::ShaderModule vertexShaderModule = this->createShaderModule(vertexShaderCode);
    vk::ShaderModule fragmentShaderModule = this->createShaderModule(fragShaderCode);
    VulkanDbg::registerVkObjectDbgInfo("ShaderModule VertexShader", vk::ObjectType::eShaderModule, reinterpret_cast<uint64_t>(vk::ShaderModule::CType(vertexShaderModule)));
    VulkanDbg::registerVkObjectDbgInfo("ShaderModule fragmentShader", vk::ObjectType::eShaderModule, reinterpret_cast<uint64_t>(vk::ShaderModule::CType(fragmentShaderModule)));

    // --- SHADER STAGE CREATION INFORMATION ---
    // Vertex Stage Creation Information
    vk::PipelineShaderStageCreateInfo vertexShaderStageCreateInfo {};
    vertexShaderStageCreateInfo.setStage(vk::ShaderStageFlagBits::eVertex);                             // Shader stage name
    vertexShaderStageCreateInfo.setModule(vertexShaderModule);                                    // Shader Modual to be used by Stage
    vertexShaderStageCreateInfo.setPName("main");                                                 //Name of the vertex Shaders main function (function to run)

    // Fragment Stage Creation Information
    vk::PipelineShaderStageCreateInfo fragmentShaderPipelineCreatInfo {};
    fragmentShaderPipelineCreatInfo.setStage(vk::ShaderStageFlagBits::eFragment);                       // Shader Stage Name
    fragmentShaderPipelineCreatInfo.setModule(fragmentShaderModule);                              // Shader Module used by stage
    fragmentShaderPipelineCreatInfo.setPName("main");                                             // name of the fragment shader main function (function to run)
    
    // Put shader stage creation infos into array
    // graphics pipeline creation info requires array of shader stage creates
    std::array<vk::PipelineShaderStageCreateInfo, 2> pipelineShaderStageCreateInfos {
            vertexShaderStageCreateInfo,
            fragmentShaderPipelineCreatInfo
    };

    // -- FIXED SHADER STAGE CONFIGURATIONS -- 

    // How the data for a single vertex (including info such as position, colour, texture coords, normals, etc)
    // is as a whole.
    vk::VertexInputBindingDescription bindingDescription;
    bindingDescription.setBinding(uint32_t (0));                 // Can bind multiple streams of data, this defines which one.
    bindingDescription.setStride(sizeof(Vertex));    // Size of a single Vertex Object
    bindingDescription.setInputRate(vk::VertexInputRate::eVertex); // How to move between data after each vertex...
    
    // How the Data  for an attribute is definied within a vertex    
    std::array<vk::VertexInputAttributeDescription, 3> attributeDescriptions{}; 

    // Position Attribute: 
    attributeDescriptions[0].setBinding(uint32_t (0));                           // which binding the data is at (should be same as above)
    attributeDescriptions[0].setLocation(uint32_t (0));                           // Which Location in shader where data will be read from
    attributeDescriptions[0].setFormat(vk::Format::eR32G32B32Sfloat);  // Format the data will take (also helps define size of data)
    attributeDescriptions[0].setOffset(offsetof(Vertex, pos));       // Sets the offset of our struct member Pos (where this attribute is defined for a single vertex...)

    // Color Attribute.
    attributeDescriptions[1].setBinding(uint32_t (0));                         
    attributeDescriptions[1].setLocation(uint32_t (1));                         
    attributeDescriptions[1].setFormat(vk::Format::eR32G32B32Sfloat);
    attributeDescriptions[1].setOffset(offsetof(Vertex, col));

    // Texture Coorinate Attribute (uv): 
    attributeDescriptions[2].setBinding(uint32_t (0));
    attributeDescriptions[2].setLocation(uint32_t (2));
    attributeDescriptions[2].setFormat(vk::Format::eR32G32Sfloat);      // Note; only RG, since it's a 2D image we don't use the depth and thus we only need RG and not RGB
    attributeDescriptions[2].setOffset(offsetof(Vertex, tex));


    // -- VERTEX INPUT --
    vk::PipelineVertexInputStateCreateInfo vertexInputCreateInfo;
    vertexInputCreateInfo.setVertexBindingDescriptionCount(uint32_t (1));
    vertexInputCreateInfo.setPVertexBindingDescriptions(&bindingDescription);
    vertexInputCreateInfo.setVertexAttributeDescriptionCount(static_cast<uint32_t>(attributeDescriptions.size()));
    vertexInputCreateInfo.setPVertexAttributeDescriptions(attributeDescriptions.data());      

    // -- INPUT ASSEMBLY --
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo;
    inputAssemblyStateCreateInfo.setTopology(vk::PrimitiveTopology::eTriangleList);        // Primitive type to assemble primitives from ;
    inputAssemblyStateCreateInfo.setPrimitiveRestartEnable(VK_FALSE);                     // Allow overrideing tof "strip" topology to start new primitives

    // -- VIEWPORT & SCISSOR ---
    // Create viewport info struct
    vk::Viewport viewport;    
    viewport.setX(0.0F);        // x start coordinate
    viewport.setY(0.0F);        // y start coordinate
    viewport.setWidth(static_cast<float>(this->swapchain.getWidth()));       // width of viewport
    viewport.setHeight(static_cast<float>(swapchain.getHeight()));     // height of viewport
    viewport.setMinDepth(0.0F);     // min framebuffer depth
    viewport.setMaxDepth(1.0F);     // max framebuffer depth
    //create a scissor info struct
    vk::Rect2D scissor;
    scissor.offset = vk::Offset2D{0, 0};                            // Offset to use Region from
    scissor.extent = this->swapchain.getVkExtent();                   // Extent to sdescribe region to use, starting at offset

    vk::PipelineViewportStateCreateInfo viewportStateCreateInfo = {};
    viewportStateCreateInfo.setViewportCount(uint32_t(1));
    viewportStateCreateInfo.setPViewports(&viewport);
    viewportStateCreateInfo.setScissorCount(uint32_t(1));
    viewportStateCreateInfo.setPScissors(&scissor);


    // --- DYNAMIC STATES ---
    // dynamic states to enable... NOTE: to change some stuff you might need to reset it first... example viewport...
    /*std::vector<VkDynamicState > dynamicStateEnables;
    dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT); // Dynamic Viewport    : can resize in command buffer with vkCmdSetViewport(command, 0,1,&viewport)
    dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);  // Dynamic Scissor     : Can resize in commandbuffer with vkCmdSetScissor(command, 0,1, &viewport)

    // Dynamic state creation info
    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
    dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
    dynamicStateCreateInfo.pDynamicStates = dynamicStateEnables.data();*/

    // --- RASTERIZER ---
    vk::PipelineRasterizationStateCreateInfo rasterizationStateCreateInfo;
    rasterizationStateCreateInfo.setDepthClampEnable(VK_FALSE);         // Change if fragments beyond near/far planes are clipped / clamped  (Requires depthclamp as a device features...)
    rasterizationStateCreateInfo.setRasterizerDiscardEnable(VK_FALSE);// Wether to discard data or skip rasterizzer. Never creates fragments, only suitable for pipleine without framebuffer output
    rasterizationStateCreateInfo.setPolygonMode(vk::PolygonMode::eFill);// How to handle filling point betweeen vertices...
    rasterizationStateCreateInfo.setLineWidth(1.F);                  // how thiock lines should be when drawn (other than 1 requires device features...)
    rasterizationStateCreateInfo.setCullMode(vk::CullModeFlagBits::eBack);  // Which face of tri to cull
    rasterizationStateCreateInfo.setFrontFace(vk::FrontFace::eCounterClockwise);// Since our Projection matrix now has a inverted Y-axis (GLM is right handed, but vulkan is left handed)
                                                                    // winding order to determine which side is front
    rasterizationStateCreateInfo.setDepthBiasEnable(VK_FALSE);        // Wether to add depthbiaoas to fragments (to remove shadowacne...)

    // --- MULTISAMPLING ---
    vk::PipelineMultisampleStateCreateInfo multisampleStateCreateInfo;
    multisampleStateCreateInfo.setSampleShadingEnable(VK_FALSE);                      // Enable multisample shading or not
    multisampleStateCreateInfo.setRasterizationSamples(vk::SampleCountFlagBits::e1);        // number of samples to use per fragment


    // --- BLENDING --
    
    // Blend attatchment State (how blending is handled)
    vk::PipelineColorBlendAttachmentState  colorState;
    colorState.setColorWriteMask(vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eB // Colours to apply blending to
                | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eR);
    colorState.setBlendEnable(VK_TRUE);                                                       // enable blending

    // Blending uses Equation: (srcColorBlendFactor * new Colour) colorBlendOp (dstColorBlendFactor * old Colour)
    colorState.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha);
    colorState.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha);
    colorState.setColorBlendOp(vk::BlendOp::eAdd);

    colorState.setSrcAlphaBlendFactor(vk::BlendFactor::eOne);
    colorState.setDstAlphaBlendFactor(vk::BlendFactor::eZero);
    colorState.setAlphaBlendOp(vk::BlendOp::eAdd);

    vk::PipelineColorBlendStateCreateInfo colorBlendingCreateInfo;
    colorBlendingCreateInfo.setLogicOpEnable(VK_FALSE);               // Alternative to calculations is to use logical Operations
    colorBlendingCreateInfo.setAttachmentCount(uint32_t (1));
    colorBlendingCreateInfo.setPAttachments(&colorState);

    // --- PIPELINE LAYOUT ---

    // We have two Descriptor Set Layouts, 
    // One for View and Projection matrices Uniform Buffer, and the other one for the texture sampler!
    std::array<vk::DescriptorSetLayout, 2> descriptorSetLayouts
    {
        this->descriptorSetLayout,
        this->samplerDescriptorSetLayout
    };

    vk::PipelineLayoutCreateInfo  pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.setSetLayoutCount(static_cast<uint32_t>(descriptorSetLayouts.size()));
    pipelineLayoutCreateInfo.setPSetLayouts(descriptorSetLayouts.data());
    pipelineLayoutCreateInfo.setPushConstantRangeCount(uint32_t(1));                    // We only have One Push Constant range we want to use
    pipelineLayoutCreateInfo.setPPushConstantRanges(&this->pushConstantRange);// the Push Constant Range we want to use

    // --- DEPTH STENCIL TESING ---
    vk::PipelineDepthStencilStateCreateInfo depthStencilCreateInfo{};
    depthStencilCreateInfo.setDepthTestEnable(VK_TRUE);              // Enable Depth Testing; Check the Depth to determine if it should write to the fragment
    depthStencilCreateInfo.setDepthWriteEnable(VK_TRUE);              // enable writing to Depth Buffer; To make sure it replaces old values
    depthStencilCreateInfo.setDepthCompareOp(vk::CompareOp::eLess);   // Describes that we want to replace the old values IF new values are smaller/lesser.
    depthStencilCreateInfo.setDepthBoundsTestEnable(VK_FALSE);             // In case we want to use as Min and a Max Depth; if depth Values exist between two bounds... 
    depthStencilCreateInfo.setStencilTestEnable(VK_FALSE);             // Whether to enable the Stencil Test; we dont use it so we let it be disabled

    this->pipelineLayout = this->getVkDevice().createPipelineLayout(pipelineLayoutCreateInfo);
    VulkanDbg::registerVkObjectDbgInfo("VkPipelineLayout GraphicsPipelineLayout", vk::ObjectType::ePipelineLayout, reinterpret_cast<uint64_t>(vk::PipelineLayout::CType(this->pipelineLayout)));
    // --- Dynamic Rendering ---
    vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo{};
    pipelineRenderingCreateInfo.setColorAttachmentCount(uint32_t (1));
    pipelineRenderingCreateInfo.setColorAttachmentFormats(this->swapchain.getVkFormat());
    pipelineRenderingCreateInfo.setDepthAttachmentFormat(this->swapchain.getVkDepthFormat());
    pipelineRenderingCreateInfo.setStencilAttachmentFormat(this->swapchain.getVkDepthFormat());

    // -- GRAPHICS PIPELINE CREATION --
    vk::GraphicsPipelineCreateInfo pipelineCreateInfo;
    pipelineCreateInfo.setStageCount(uint32_t(2));                                          // Number of shader stages
    pipelineCreateInfo.setPStages(pipelineShaderStageCreateInfos.data());         // List of Shader stages
    pipelineCreateInfo.setPVertexInputState(&vertexInputCreateInfo);              // All the fixed function Pipeline states
    pipelineCreateInfo.setPInputAssemblyState(&inputAssemblyStateCreateInfo);
    pipelineCreateInfo.setPViewportState(&viewportStateCreateInfo);
    pipelineCreateInfo.setPDynamicState(nullptr);
    pipelineCreateInfo.setPRasterizationState(&rasterizationStateCreateInfo);
    pipelineCreateInfo.setPMultisampleState(&multisampleStateCreateInfo);
    pipelineCreateInfo.setPColorBlendState(&colorBlendingCreateInfo);
    pipelineCreateInfo.setPDepthStencilState(&depthStencilCreateInfo);
    pipelineCreateInfo.setLayout(this->pipelineLayout);                                 // Pipeline layout pipeline should use
    pipelineCreateInfo.setPNext(&pipelineRenderingCreateInfo);

    pipelineCreateInfo.setRenderPass(nullptr);                                 // Render pass description the pipeline is compatible with
    pipelineCreateInfo.setSubpass(uint32_t(0));                                             // subpass of render pass to use with pipeline

    // Pipeline Derivatives : Can Create multiple pipelines that derive from one another for optimization
    pipelineCreateInfo.setBasePipelineHandle(nullptr); // Existing pipeline to derive from ...
    pipelineCreateInfo.setBasePipelineIndex(int32_t(-1));              // or index of pipeline being created to derive from ( in case of creating multiple of at once )

    // Create Graphics Pipeline
    vk::Result result{};
    std::tie(result , this->graphicsPipeline) = getVkDevice().createGraphicsPipeline(graphics_pipelineCache,pipelineCreateInfo);  
    if(result != vk::Result::eSuccess)
    {
        Log::error("Could not create Pipeline");
    }
    VulkanDbg::registerVkObjectDbgInfo("VkPipeline GraphicsPipeline", vk::ObjectType::ePipeline, reinterpret_cast<uint64_t>(vk::Pipeline::CType(this->graphicsPipeline)));

    // Check cache header to validate if cache is ok
    uint32_t headerLength = 0;
    uint32_t cacheHeaderVersion = 0;
    uint32_t vendorID = 0;
    uint32_t deviceID = 0;
    uint8_t pipelineCacheUUID[VK_UUID_SIZE] = {};

    std::vector<uint8_t> loaded_cache;
    loaded_cache = this->getVkDevice().getPipelineCacheData(graphics_pipelineCache);

    memcpy(&headerLength, (uint8_t *)loaded_cache.data() + 0, 4);
    memcpy(&cacheHeaderVersion, (uint8_t *)loaded_cache.data() + 4, 4);
    memcpy(&vendorID, (uint8_t *)loaded_cache.data() + 8, 4);
    memcpy(&deviceID, (uint8_t *)loaded_cache.data() + 12, 4);
    memcpy(pipelineCacheUUID, (uint8_t *)loaded_cache.data() + 16, VK_UUID_SIZE);

    std::ofstream save_cache(pipeline_cache_name, std::ios::binary);

    save_cache.write((char*)loaded_cache.data(), static_cast<uint32_t>(loaded_cache.size()));
    save_cache.close();

    //Destroy Shader Moduels, no longer needed after pipeline created
    this->getVkDevice().destroyShaderModule(vertexShaderModule);
    this->getVkDevice().destroyShaderModule(fragmentShaderModule);
    }
}

vk::ShaderModule VulkanRenderer::createShaderModule(const std::vector<char> &code)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    vk::ShaderModuleCreateInfo shaderCreateInfo = {};
    shaderCreateInfo.setCodeSize(code.size());                                    // Size of code
    shaderCreateInfo.setPCode(reinterpret_cast<const uint32_t*>(code.data()));    // pointer of code (of uint32_t pointe rtype //NOLINT:Ok to use Reinterpret cast here

    vk::ShaderModule shaderModule = this->getVkDevice().createShaderModule(shaderCreateInfo);
    return shaderModule;
}

int VulkanRenderer::createTextureImage(const std::string &filename)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    // Load the image file
    int width = 0;
    int height = 0;
    vk::DeviceSize imageSize = 0;
    stbi_uc* imageData = this->loadTextuerFile(filename, &width,&height, &imageSize);

    //Create Staging buffer to hold loaded data, ready to copy to device
    vk::Buffer imageStagingBuffer = nullptr;
    //vk::DeviceMemory imageStagingBufferMemory = nullptr;
    VmaAllocation imageStagingBufferMemory = nullptr;
    VmaAllocationInfo allocInfo;

    vengine_helper::createBuffer(
        {
            .physicalDevice = this->physicalDevice.getVkPhysicalDevice(),
            .device = this->getVkDevice(), 
            .bufferSize = imageSize, 
            .bufferUsageFlags = vk::BufferUsageFlagBits::eTransferSrc, 
            // .bufferProperties = vk::MemoryPropertyFlagBits::eHostVisible         // Staging buffer needs to be visible from HOST  (CPU), in order for modification
            //                     |   vk::MemoryPropertyFlagBits::eHostCoherent,   // not using cache...
            .bufferProperties = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                                | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .buffer = &imageStagingBuffer, 
            .bufferMemory = &imageStagingBufferMemory,
            .allocationInfo = &allocInfo,
            .vma = &vma
        });

    void* data = nullptr; 
    
    if(vmaMapMemory(this->vma, imageStagingBufferMemory, &data) != VK_SUCCESS)
    {
        Log::error("Failed to allocate Mesh Staging Texture Image Buffer Using VMA!");
    }

    memcpy(data, imageData, imageSize);
    vmaUnmapMemory(this->vma , imageStagingBufferMemory);
    

    // Free image data allocated through stb_image.h 
    stbi_image_free(imageData);

    // Create image to hold final texture
    vk::Image texImage = nullptr;
    //vk::DeviceMemory texImageMemory = nullptr;
    VmaAllocation texImageMemory = nullptr;
    texImage = Texture::createImage(
        this->vma,
        {
            .width = static_cast<uint32_t>(width), 
            .height = static_cast<uint32_t>(height), 
            .format = vk::Format::eR8G8B8A8Unorm,               // use Alpha channel even if image does not have... 
            .tiling =vk::ImageTiling::eOptimal,                // Same value as the Depth Buffer uses (Dont know if it has to be)
            .useFlags = vk::ImageUsageFlagBits::eTransferDst         // Data should be transfered to the GPU, from the staging buffer
                        |   vk::ImageUsageFlagBits::eSampled,         // This image will be Sampled by a Sampler!                         
            .property = vk::MemoryPropertyFlagBits::eDeviceLocal,    // Image should only be accesable on the GPU 
            .imageMemory = &texImageMemory
        },
        filename // Describing what image is being created, for debug purposes...
    );

    // - COPY THE DATA TO THE IMAGE -
    // Transition image to be in the DST, needed by the Copy Operation (Copy assumes/needs image Layout to be in vk::ImageLayout::eTransferDstOptimal state)
    vengine_helper::transitionImageLayout(
        this->getVkDevice(), 
        this->queueFamilies.getGraphicsQueue(),
        this->graphicsCommandPool, 
        texImage,                               // Image to transition the layout on
        vk::ImageLayout::eUndefined,              // Image Layout to transition the image from
        vk::ImageLayout::eTransferDstOptimal);  // Image Layout to transition the image to

    // Copy Data to image
    vengine_helper::copyImageBuffer(
        this->getVkDevice(), 
        this->queueFamilies.getGraphicsQueue(),
        this->graphicsCommandPool, 
        imageStagingBuffer, 
        texImage, 
        width, 
        height
    );

    // Transition iamge to be shader readable for shader usage
    vengine_helper::transitionImageLayout(
        this->getVkDevice(), 
        this->queueFamilies.getGraphicsQueue(),
        this->graphicsCommandPool, 
        texImage,
        vk::ImageLayout::eTransferDstOptimal,       // Image layout to transition the image from; this is the same as we transition the image too before we copied buffer!
        vk::ImageLayout::eShaderReadOnlyOptimal);  // Image Layout to transition the image to; in order for the Fragment Shader to read it!         

    // Add texture data to vector for reference 
    textureImages.push_back(texImage);
    textureImageMemory.push_back(texImageMemory);

    // Destroy and Free the staging buffer + staging buffer memroy
    this->getVkDevice().destroyBuffer(imageStagingBuffer);
    //this->getVkDevice().freeMemory(imageStagingBufferMemory);
    vmaFreeMemory(this->vma, imageStagingBufferMemory);

    // Return index of last pushed image!
    return static_cast<int>(textureImages.size()) -1; 
}

int VulkanRenderer::createTexture(const std::string &filename)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    // Create Texture Image and get its Location in array
    int textureImageLoc = createTextureImage(filename);

    // Create Image View
    vk::ImageView imageView = Texture::createImageView(
        this->device,
        this->textureImages[textureImageLoc],   // The location of the Image in our textureImages vector
        vk::Format::eR8G8B8A8Unorm,               // Format for rgba 
        vk::ImageAspectFlagBits::eColor
    );

    // Add the Image View to our vector with Image views
    this->textureImageViews.push_back(imageView);

    // Create Texture Descriptor
    int descriptorLoc = createSamplerDescriptor(imageView);

    // Return index of Texture Descriptor that was just created
    return descriptorLoc;
}

int VulkanRenderer::createSamplerDescriptor(vk::ImageView textureImage)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    // Descriptor Set Allocation Info
    vk::DescriptorSetAllocateInfo setAllocateInfo;
    setAllocateInfo.setDescriptorPool(this->samplerDescriptorPool);
    setAllocateInfo.setDescriptorSetCount(uint32_t (1));
    setAllocateInfo.setPSetLayouts(&this->samplerDescriptorSetLayout);

    // Allocate Descriptor Sets
    vk::DescriptorSet descriptorSet = 
        this->getVkDevice().allocateDescriptorSets(setAllocateInfo)[0];

    // Tedxture Image info
    vk::DescriptorImageInfo imageInfo;
    imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);     // The Image Layout when it is in use
    imageInfo.setImageView(textureImage);                                 // Image to be bind to set
    imageInfo.setSampler(this->textureSampler);                         // the Sampler to use for this Descriptor Set

    // Descriptor Write Info
    vk::WriteDescriptorSet descriptorWrite;
    descriptorWrite.setDstSet(descriptorSet);
    descriptorWrite.setDstArrayElement(uint32_t (0));
    descriptorWrite.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
    descriptorWrite.setDescriptorCount(uint32_t (1));
    descriptorWrite.setPImageInfo(&imageInfo);

    // Update the new Descriptor Set
    this->getVkDevice().updateDescriptorSets(
        uint32_t(1),
        &descriptorWrite,
        uint32_t(0),
        nullptr
    );

    // Add descriptor Set to our list of descriptor Sets
    this->samplerDescriptorSets.push_back(descriptorSet);

    //Return the last created Descriptor set
    return static_cast<int>(this->samplerDescriptorSets.size() - 1); 
}

int VulkanRenderer::createModel(const std::string& modelFile)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif    
    // Import Model Scene
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(
        (DEF<std::string>(P_MODELS)+modelFile).c_str(),
        aiProcess_Triangulate               // Ensures that ALL objects will be represented as Triangles
        | aiProcess_FlipUVs                 // Flips the texture UV values, to be same as how we use them
        | aiProcess_JoinIdenticalVertices   // Saves memory by making sure no dublicate vertices exists
        );

    if(scene == nullptr)
    {
        Log::error("Failed to load model ("+modelFile+")");
    }

    // Get vector of all materials 
    std::vector<std::string> textureNames = Model::loadMaterials(scene);

    // Handle empty texture 
    std::vector<int> matToTexture(textureNames.size());

    for(size_t i = 0; i < textureNames.size(); i++){
        
        if(textureNames[i].empty())
        {
            matToTexture[i] = 0; // Use default textures for models if textures are missing
        }
        else
        {
            // Create texture, use the index returned by our createTexture function
            matToTexture[i] = createTexture(textureNames[i]);
        }
    }

    // Load in all meshes
    std::vector<Mesh> modelMeshes = Model::getMeshesFromNodeTree(
        &this->vma,
        this->physicalDevice.getVkPhysicalDevice(), 
        this->getVkDevice(), 
        this->queueFamilies.getGraphicsQueue(),
        this->graphicsCommandPool, 
        scene, 
        matToTexture
    );

    // Create Model, add to list
    Model model = Model(
        &this->vma,
        this->physicalDevice.getVkPhysicalDevice(), 
        this->getVkDevice(), 
        this->queueFamilies.getGraphicsQueue(),
        this->graphicsCommandPool,
        modelMeshes
    );
    modelList.emplace_back(model);

    return static_cast<int>(modelList.size())-1;

}

stbi_uc* VulkanRenderer::loadTextuerFile(const std::string &filename, int* width, int* height, vk::DeviceSize* imageSize)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    // Number of Channels the image uses, will not be used but could be used in the future
    int channels = 0;
    using namespace vengine_helper::config;
    // Load pixel Data from file to image
    std::string fileLoc  = DEF<std::string>(P_TEXTURES) + filename;
    stbi_uc* image = stbi_load(
            fileLoc.c_str(),
            width,
            height,
            &channels,          // In case we want to  use channels, its stored in channels
            STBI_rgb_alpha );   // force image to be in format : RGBA

    if(image == nullptr)
    {
        Log::error("Failed to load a Texture file! ("+filename+")");
    }
    

    // Calculate image sisze using given and known data
    *imageSize = static_cast<uint32_t>((*width) * (*height) * 4); // width times height gives us size per channel, we have 4 channels! (rgba)

    return image;
    
}

VulkanRenderer::VulkanRenderer()
    : window(nullptr)
{
    loadConfIntoMemory();
}

void VulkanRenderer::createRenderPass_Base() 
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    // Array of our subPasses    
    std::array<vk::SubpassDescription2, 2> subPasses {};     

    // - ATTATCHMENTS - 
    // SUBPASS 1 ATTACHMENTS (Input attachments) and ATTACHMENT REFERENCES

    // Color Attachment (Input)
    vk::AttachmentDescription2 colorAttachment {};
    colorAttachment.setFormat(this->swapchain.getVkColorFormat());        
        
    colorAttachment.setSamples(vk::SampleCountFlagBits::e1);
    colorAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);      // When we start the renderpass, first thing to do is to clear since there is no values in it yet
    colorAttachment.setStoreOp(vk::AttachmentStoreOp::eStore); // How to store it after the RenderPass; We dont care! But we do care what happens after the first SubPass! (not handled here)
    colorAttachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
    colorAttachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
    colorAttachment.setInitialLayout(vk::ImageLayout::eUndefined);        // We dont care what the image layout is when we start. But we do care about what layout it is when it enter the first SubPass! (not handled here)
    //colorAttachment.setFinalLayout(vk::ImageLayout::eAttachmentOptimal); // Should be the same value as it was after the subpass finishes (??)
    colorAttachment.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal); //(!!) Should be the same value as it was after the subpass finishes (??)

    

    // Depth Attatchment Of Render Pass 
    vk::AttachmentDescription2 depthAttatchment{};
    depthAttatchment.setFormat(this->swapchain.getVkDepthFormat());
    depthAttatchment.setSamples(vk::SampleCountFlagBits::e1);
    depthAttatchment.setLoadOp(vk::AttachmentLoadOp::eClear);      // Clear Buffer Whenever we try to load data into (i.e. clear before use it!)
    depthAttatchment.setStoreOp(vk::AttachmentStoreOp::eDontCare); // Whenever it's used, we dont care what happens with the data... (we dont present it or anything)
    depthAttatchment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);  // Even though the Stencil i present, we dont plan to use it. so we dont care    
    depthAttatchment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);     // Even though the Stencil i present, we dont plan to use it. so we dont care
    depthAttatchment.setInitialLayout(vk::ImageLayout::eUndefined);        // We don't care how the image layout is initially, so let it be undefined
    depthAttatchment.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal); // Final layout should be Optimal for Depth Stencil attachment!

    // Color Attachment (input) Reference
    vk::AttachmentReference2 colorAttachmentReference {};    
    colorAttachmentReference.setAttachment(uint32_t(1));            // Match the number/ID of the Attachment to the index of the FrameBuffer!
    colorAttachmentReference.setLayout(vk::ImageLayout::eColorAttachmentOptimal); // The Layout the Subpass must be in! 

    // Depth Attachment (input) Reference
    vk::AttachmentReference2 depthAttachmentReference {};
    depthAttachmentReference.setAttachment(uint32_t(2)); 
    depthAttachmentReference.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal); // The layout the subpass must be in! Should be same as 'final layout'(??)

    // Setup Subpass 1
    subPasses[0].setPipelineBindPoint(vk::PipelineBindPoint::eGraphics); 
    subPasses[0].setColorAttachmentCount(uint32_t(1)); 
    subPasses[0].setPColorAttachments(&colorAttachmentReference); 
    subPasses[0].setPDepthStencilAttachment(&depthAttachmentReference); 

    // SUBPASS 2 ATTACHMENTS and ATTACHMENT REFERENCES

    // Color Attachment SwapChain
    vk::AttachmentDescription2 swapchainColorAttachment{};
    swapchainColorAttachment.setFormat(this->swapchain.getVkFormat());              // Format to use for attachgment
    swapchainColorAttachment.setSamples(vk::SampleCountFlagBits::e1);            // Number of samples to write for multisampling
    swapchainColorAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);       // Descripbes what to do with attachment before rendeing
    swapchainColorAttachment.setStoreOp(vk::AttachmentStoreOp::eStore);     // Describes what to do with Attachment after rendering
    swapchainColorAttachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);     // Describes what to do with stencil before rendering
    swapchainColorAttachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);   // Describes what to do with stencil affter rendering

    // Framebuffer data will be stored as an image, but images can be given different data layouts
    // to give optimal usefor certain operations
    swapchainColorAttachment.setInitialLayout(vk::ImageLayout::eUndefined);      // image data layout before render pass starts
    //swapchainColorAttachment.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);  // Image data layout after render pass (to change to)
    swapchainColorAttachment.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);  // edit: After adding Imgui, this renderpass will no longer be the presenting one, thus we change final layout! 

    vk::AttachmentReference2 swapchainColorAttachmentReference = {};
    swapchainColorAttachmentReference.setAttachment(uint32_t(0));                                       // Attatchment on index 0 of array
    swapchainColorAttachmentReference.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    // Create two more Attachment References; These describes what the subpass will take input from
    std::array<vk::AttachmentReference2,2 > inputReferences{};
    inputReferences[0].setAttachment(uint32_t(1));                                        // The Attachment index (Color Input)
    inputReferences[0].setLayout(vk::ImageLayout::eShaderReadOnlyOptimal); // reuse the attachment (1), make sure it's uses SHADER_READ_ONLY_OPTIMAL layout!
    inputReferences[0].setAspectMask(vk::ImageAspectFlagBits::eColor);//(!!)
    inputReferences[0].setPNext(nullptr); //(!!)(??)
    inputReferences[1].setAttachment(uint32_t(2));                                        // The Attachment index (Depth Input)
    inputReferences[1].setAspectMask(vk::ImageAspectFlagBits::eDepth);//(!!)    
    inputReferences[1].setLayout(vk::ImageLayout::eReadOnlyOptimal); // reuse the attachment (2), make sure it's uses SHADER_READ_ONLY_OPTIMAL layout!
    inputReferences[1].setPNext(nullptr); //(!!)(??)

    // Setup Subpass 2
    subPasses[1].setPipelineBindPoint(vk::PipelineBindPoint::eGraphics); 
    subPasses[1].setColorAttachmentCount(uint32_t(1)); 
    subPasses[1].setPColorAttachments(&swapchainColorAttachmentReference); 
    subPasses[1].setInputAttachmentCount(static_cast<uint32_t>(inputReferences.size()));    // How many Inputs Subpass 2 will have
    subPasses[1].setPInputAttachments(inputReferences.data());                           // The Input Attachments References Subpass 2 will use
    
    // Replacing all subpass dependencies with this results in no errors/warnings. But I dont know if it is correct... Seems to be the same if subpassDependency[0] and [2] are removed from the code above...
    std::array<vk::SubpassDependency2, 1> subpassDependencies{};
    subpassDependencies[0].setSrcSubpass(0);
    subpassDependencies[0].setDstSubpass(1);
    subpassDependencies[0].setSrcStageMask(vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests);
    subpassDependencies[0].setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader);
    subpassDependencies[0].setSrcAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite);
    subpassDependencies[0].setDstAccessMask(vk::AccessFlagBits::eInputAttachmentRead);
    subpassDependencies[0].setDependencyFlags(vk::DependencyFlagBits::eByRegion);

    // Vector with the Attatchments
    std::array<vk::AttachmentDescription2,3> attachments
    {
        swapchainColorAttachment,   // Attachment on index 0 of array : SwapChain Color
        colorAttachment,            // Attachment on index 1 of array : Color (input to SubPass 2)
        depthAttatchment            // Attachment on index 2 of array : Depth (input to SubPass 2)
    };

    //Create info for render pass
    vk::RenderPassCreateInfo2 renderPassCreateInfo;
    renderPassCreateInfo.setAttachmentCount(static_cast<uint32_t>(attachments.size()));
    renderPassCreateInfo.setPAttachments(attachments.data());
    renderPassCreateInfo.setSubpassCount(static_cast<uint32_t>(subPasses.size()));
    renderPassCreateInfo.setPSubpasses(subPasses.data());
    renderPassCreateInfo.setDependencyCount(static_cast<uint32_t> (subpassDependencies.size()));
    renderPassCreateInfo.setPDependencies(subpassDependencies.data());

    this->renderPass_base = this->getVkDevice().createRenderPass2(renderPassCreateInfo);

    VulkanDbg::registerVkObjectDbgInfo("The RenderPass", vk::ObjectType::eRenderPass, reinterpret_cast<uint64_t>(vk::RenderPass::CType(this->renderPass_base)));
    
}

void VulkanRenderer::createRenderPass_Imgui()
{
    vk::AttachmentDescription2 attachment{};
    attachment.setFormat(this->swapchain.getVkFormat());
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
    this->renderPass_imgui = this->getVkDevice().createRenderPass2(renderpassCreateinfo);
}

void VulkanRenderer::createDescriptorSetLayout()
{    
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    // - CREATE UNIFORM VALUES DESCRIPTOR SET LAYOUT -

    // UboViewProjection binding Info
    vk::DescriptorSetLayoutBinding vpLayoutBinding;
    vpLayoutBinding.setBinding(uint32_t (0));                                           // Describes which Binding Point in the shaders this layout is being bound to
    vpLayoutBinding.setDescriptorType(vk::DescriptorType::eUniformBuffer);    // Type of descriptor (Uniform, Dynamic uniform, image Sampler, etc.)
    vpLayoutBinding.setDescriptorCount(uint32_t(1));                                   // Amount of actual descriptors we're binding, where just binding one; our MVP struct
    vpLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eVertex);               // What Shader Stage we want to bind our Descriptor set to
    vpLayoutBinding.setPImmutableSamplers(nullptr);//vknullhandle??          // Used by Textures; whether or not the Sampler should be Immutable

    // Adding the Bindings to a Vector in order to submit all the DescriptorSetLayout Bindings! 
    std::vector<vk::DescriptorSetLayoutBinding> layoutBindings {
        vpLayoutBinding
        // Left for Reference; we dont use Dynamic Uniform Buffers for our Model Matrix anymore.
        //,modelLayoutBinding
    };

    // Create Descriptor Set Layout with given bindings
    vk::DescriptorSetLayoutCreateInfo layoutCreateInfo{};
    layoutCreateInfo.setBindingCount(static_cast<uint32_t>(layoutBindings.size()));  // Number of Binding infos
    layoutCreateInfo.setPBindings(layoutBindings.data());                            // Array containing the binding infos

    // Create Descriptor Set Layout
    this->descriptorSetLayout = this->getVkDevice().createDescriptorSetLayout(layoutCreateInfo);
    
    VulkanDbg::registerVkObjectDbgInfo("DescriptorSetLayout ViewProjection", vk::ObjectType::eDescriptorSetLayout, reinterpret_cast<uint64_t>(vk::DescriptorSetLayout::CType(this->descriptorSetLayout)));

    // - CREATE TEXTURE SAMPLER DESCRIPTOR SET LAYOUT -
    // Texture Binding Info
    vk::DescriptorSetLayoutBinding samplerLayoutBinding;
    samplerLayoutBinding.setBinding(uint32_t (0));                                   // This can be 0 too, as it will be for a different Descriptor Set, Descriptor set 1 (previous was Descriptor Set 0)! 
    samplerLayoutBinding.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
    samplerLayoutBinding.setDescriptorCount(uint32_t (1));               
    samplerLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eFragment);     // The Stage the descriptor layout will pass to will be the Fragment Shader
    samplerLayoutBinding.setPImmutableSamplers(nullptr);

    // Create a Descriptor Set Layout with given bindings for texture
    vk::DescriptorSetLayoutCreateInfo textureLayoutCreateInfo;
    textureLayoutCreateInfo.setBindingCount(uint32_t (1));
    textureLayoutCreateInfo.setPBindings(&samplerLayoutBinding);

    // create Descriptor Set Layout
    this->samplerDescriptorSetLayout = this->getVkDevice().createDescriptorSetLayout(textureLayoutCreateInfo);
    VulkanDbg::registerVkObjectDbgInfo("DescriptorSetLayout SamplerTexture", vk::ObjectType::eDescriptorSetLayout, reinterpret_cast<uint64_t>(vk::DescriptorSetLayout::CType(this->samplerDescriptorSetLayout)));

    // CREATE INPUT ATTACHMENT IMAGE DESCRIPTOR SET LAYOUT
    // Colour Input Binding
    vk::DescriptorSetLayoutBinding colorInputLayoutBinding;
    colorInputLayoutBinding.setBinding(uint32_t (0));                                            // Binding 0 for Set 0, but for a new pipeline
    colorInputLayoutBinding.setDescriptorCount(uint32_t (1));
    colorInputLayoutBinding.setDescriptorType(vk::DescriptorType::eInputAttachment);   // Describes that the Descriptor is used by Input Attachment
    colorInputLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eFragment);              // Passed into the Fragment Shader Stage

    vk::DescriptorSetLayoutBinding depthInputLayoutBinding;
    depthInputLayoutBinding.setBinding(uint32_t (1));
    depthInputLayoutBinding.setDescriptorCount(uint32_t (1)); 
    depthInputLayoutBinding.setDescriptorType(vk::DescriptorType::eInputAttachment);
    depthInputLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eFragment);

    std::array<vk::DescriptorSetLayoutBinding, 2> inputLayoutBindings {   // Bindings for the Second Subpass input attachments (??)
        colorInputLayoutBinding,
        depthInputLayoutBinding
    };

    // Create a Descriptor Set Layout for input attachments
    vk::DescriptorSetLayoutCreateInfo inputCreateInfo;
    inputCreateInfo.setBindingCount(static_cast<uint32_t>(inputLayoutBindings.size()));
    inputCreateInfo.setPBindings(inputLayoutBindings.data());
    inputCreateInfo.setFlags(vk::DescriptorSetLayoutCreateFlags());    
    inputCreateInfo.pNext = nullptr;

    // create Descriptor Set Layout
    this->inputSetLayout = this->getVkDevice().createDescriptorSetLayout(inputCreateInfo);
    VulkanDbg::registerVkObjectDbgInfo("DescriptorSetLayout SubPass2_input", vk::ObjectType::eDescriptorSetLayout, reinterpret_cast<uint64_t>(vk::DescriptorSetLayout::CType(this->inputSetLayout)));
}

void VulkanRenderer::createPushConstantRange()
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    // Define the Push Constants values
    this->pushConstantRange.setStageFlags(vk::ShaderStageFlagBits::eVertex);    // Push Constant should be available in the Vertex Shader!
    this->pushConstantRange.setOffset(uint32_t (0));                             // Offset into the given data that our Push Constant value is (??)
    this->pushConstantRange.setSize(sizeof(ModelMatrix));                 // Size of the Data being passed
}

void VulkanRenderer::createCommandPool()
 {
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    vk::CommandPoolCreateInfo poolInfo = {};
    poolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);   // Enables us to reset our CommandBuffers 
                                                                        // if they were allocated from this CommandPool!
                                                                        // To make use of this feature you also have to activate in during Recording! (??)
    poolInfo.queueFamilyIndex = 
        this->queueFamilies.getGraphicsIndex();      // Queue family type that buffers from this command pool will use

    // Create a graphics Queue Family Command Pool
    this->graphicsCommandPool = this->getVkDevice().createCommandPool(poolInfo);
    VulkanDbg::registerVkObjectDbgInfo("CommandPool Presentation/Graphics", vk::ObjectType::eCommandPool, reinterpret_cast<uint64_t>(vk::CommandPool::CType(this->graphicsCommandPool)));

}

void VulkanRenderer::createCommandBuffers() 
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    // Resize command buffer count to have one for each framebuffer
    this->commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    vk::CommandBufferAllocateInfo cbAllocInfo;
    cbAllocInfo.setCommandPool(graphicsCommandPool);
    cbAllocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
    cbAllocInfo.setCommandBufferCount(static_cast<uint32_t>(this->commandBuffers.size()));

    // Allocate command Buffers and place handles in array of buffers
    this->commandBuffers = this->getVkDevice().allocateCommandBuffers(cbAllocInfo);

    for(size_t i = 0; i < this->commandBuffers.size(); i++)
    {
        VulkanDbg::registerVkObjectDbgInfo("Graphics CommandBuffer["+std::to_string(i)+"]", vk::ObjectType::eCommandBuffer, reinterpret_cast<uint64_t>(vk::CommandBuffer::CType(this->commandBuffers[i])));
    }
}

void VulkanRenderer::createSynchronisation()
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    // Create Semaphores for each Transition a Image can be in
    this->imageAvailable.resize(MAX_FRAMES_IN_FLIGHT);
    this->renderFinished.resize(MAX_FRAMES_IN_FLIGHT);
    this->drawFences.resize(MAX_FRAMES_IN_FLIGHT);

    //  Semaphore Creation information
    vk::SemaphoreCreateInfo semaphoreCreateInfo; 
    

    // Fence Creation Information
    vk::FenceCreateInfo fenceCreateInfo;
    fenceCreateInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);           // Make sure the Fence is initially open!

    for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){

        this->imageAvailable[i] = getVkDevice().createSemaphore(semaphoreCreateInfo);
        VulkanDbg::registerVkObjectDbgInfo("Semaphore imageAvailable["+std::to_string(i)+"]", vk::ObjectType::eSemaphore, reinterpret_cast<uint64_t>(vk::Semaphore::CType(this->imageAvailable[i])));
        
        this->renderFinished[i] = getVkDevice().createSemaphore(semaphoreCreateInfo);
        VulkanDbg::registerVkObjectDbgInfo("Semaphore renderFinished["+std::to_string(i)+"]", vk::ObjectType::eSemaphore, reinterpret_cast<uint64_t>(vk::Semaphore::CType(this->renderFinished[i])));
            
        this->drawFences[i] = getVkDevice().createFence(fenceCreateInfo);
        VulkanDbg::registerVkObjectDbgInfo("Fence drawFences["+std::to_string(i)+"]", vk::ObjectType::eFence, reinterpret_cast<uint64_t>(vk::Fence::CType(this->drawFences[i])));
    }
}

void VulkanRenderer::createTextureSampler()
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    // Sampler Creation info;
    vk::SamplerCreateInfo samplerCreateInfo;
    samplerCreateInfo.setMagFilter(vk::Filter::eLinear);                        // How the sampler will sample from a texture when it's getting closer
    samplerCreateInfo.setMinFilter(vk::Filter::eLinear);                        // How the sampler will sample from a texture when it's getting further away
    samplerCreateInfo.setAddressModeU(vk::SamplerAddressMode::eRepeat);        // How the texture will be Wrapped in U (x) direction
    samplerCreateInfo.setAddressModeV(vk::SamplerAddressMode::eRepeat);        // How the texture will be Wrapped in V (y) direction
    samplerCreateInfo.setAddressModeW(vk::SamplerAddressMode::eRepeat);        // How the texture will be Wrapped in W (z) direction
    samplerCreateInfo.setBorderColor(vk::BorderColor::eIntOpaqueBlack);      // Color of what is around the texture (in case of Repeat, it wont be used)
    samplerCreateInfo.setUnnormalizedCoordinates(VK_FALSE);                   // We want to used Normalised Coordinates (between 0 and 1), so unnormalized coordinates must be false... 
    samplerCreateInfo.setMipmapMode(vk::SamplerMipmapMode::eLinear);           // How the mipmap mode will switch between the mipmap images (interpolate between images), (we dont use it, but we set it up)
    samplerCreateInfo.setMipLodBias(0.F);                                     // Level of detail bias for mip level...
    samplerCreateInfo.setMinLod(0.F);                                     // Minimum level of Detail to pick mip level
    samplerCreateInfo.setMaxLod(VK_LOD_CLAMP_NONE);                                     // Maxiumum level of Detail to pick mip level
    samplerCreateInfo.setAnisotropyEnable(VK_TRUE);                           // Enable Anisotropy; take into account the angle of a surface is being viewed from and decide details based on that (??)
    //samplerCreateInfo.setAnisotropyEnable(VK_FALSE);                           // Disable Anisotropy; Cause Performance Issues according to validation... 
                                                                            // TODO: Check how anisotrophy can be used without causing validation errors... ? 
    samplerCreateInfo.setMaxAnisotropy(DEF<float>(SAMPL_MAX_ANISOSTROPY)); // Level of Anisotropy; 16 is a common option in the settings for alot of Games 

    this->textureSampler = this->getVkDevice().createSampler(samplerCreateInfo);
    VulkanDbg::registerVkObjectDbgInfo("Texture Sampler", vk::ObjectType::eSampler, reinterpret_cast<uint64_t>(vk::Sampler::CType(this->textureSampler)));
}

void VulkanRenderer::createUniformBuffers()
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    // viewProjection_UniformBuffer size will be size of the view and Projection Members (will offset to access)
    vk::DeviceSize viewProjection_buffer_size = sizeof(UboViewProjection);

    // One uniform buffer for each image ( and by extension, command buffer)
    this->viewProjection_uniformBuffer.resize(MAX_FRAMES_IN_FLIGHT);        // Resize to have as many ViewProjection buffers as frames in flight
    this->viewProjection_uniformBufferMemory.resize(MAX_FRAMES_IN_FLIGHT);
    this->viewProjection_uniformBufferMemory_info.resize(MAX_FRAMES_IN_FLIGHT);

    // Create Uniform Buffers 
    for(size_t i = 0; i < this->viewProjection_uniformBuffer.size(); i++)
    {
        // Create regular Uniform Buffers
        vengine_helper::createBuffer(
            {
                .physicalDevice = this->physicalDevice.getVkPhysicalDevice(),
                .device         = this->getVkDevice(), 
                .bufferSize     = viewProjection_buffer_size, 
                .bufferUsageFlags = vk::BufferUsageFlagBits::eUniformBuffer,         // We're going to use this as a Uniform Buffer...
                // .bufferProperties = vk::MemoryPropertyFlagBits::eHostVisible         // So we can access the Data from the HOST (CPU)
                //                     | vk::MemoryPropertyFlagBits::eHostCoherent,     // So we don't have to flush the data constantly...
                .bufferProperties = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                                | VMA_ALLOCATION_CREATE_MAPPED_BIT,
                .buffer         = &this->viewProjection_uniformBuffer[i], 
                .bufferMemory   = &this->viewProjection_uniformBufferMemory[i],
                .allocationInfo   = &this->viewProjection_uniformBufferMemory_info[i],
                .vma = &vma
            });

        VulkanDbg::registerVkObjectDbgInfo("ViewProjection UniformBuffer["+std::to_string(i)+"]", vk::ObjectType::eBuffer, reinterpret_cast<uint64_t>(vk::Buffer::CType(this->viewProjection_uniformBuffer[i])));
    }

}

void VulkanRenderer::createDescriptorPool()
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    // - CRTEATE UNIFORM DESCRIPTOR POOL -

    // Pool Size is definied by the Type of the Descriptors times number of those Descriptors
    // viewProjection uniform Buffer Pool size
    vk::DescriptorPoolSize viewProjection_poolSize {};
    viewProjection_poolSize.setType(vk::DescriptorType::eUniformBuffer);                                     // Descriptors in Set will be of Type Uniform Buffer    
    viewProjection_poolSize.setDescriptorCount(MAX_FRAMES_IN_FLIGHT); // How many Descriptors we want, we want One uniformBuffer so we its only the size of our uniformBuffer

    std::vector<vk::DescriptorPoolSize> descriptorPoolSizes
    {
        viewProjection_poolSize
    };

    // Data to create Descriptor Pool
    vk::DescriptorPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.setMaxSets(MAX_FRAMES_IN_FLIGHT);             // Max Nr Of descriptor Sets that can be created from the pool, 
                                                                                        // Same as the number of buffers / images we have. 
    poolCreateInfo.setPoolSizeCount(static_cast<uint32_t>(descriptorPoolSizes.size()));   // Based on how many pools we have in our descriptorPoolSizes
    poolCreateInfo.setPPoolSizes(descriptorPoolSizes.data());                          // PoolSizes to create the Descriptor Pool with

    this->descriptorPool = this->getVkDevice().createDescriptorPool(poolCreateInfo);
    VulkanDbg::registerVkObjectDbgInfo("DescriptorPool UniformBuffer ", vk::ObjectType::eDescriptorPool, reinterpret_cast<uint64_t>(vk::DescriptorPool::CType(this->descriptorPool)));
        
    // - CRTEATE SAMPLER DESCRIPTOR POOL -
    // Texture Sampler Pool
    vk::DescriptorPoolSize samplerPoolSize{};
    samplerPoolSize.setType(vk::DescriptorType::eCombinedImageSampler);       // This descriptor pool will have descriptors for Image and Sampler combined    
                                                                            // NOTE; Should be treated as seperate Concepts! but this will be enough...
    samplerPoolSize.setDescriptorCount(MAX_OBJECTS);                          // There will be as many Descriptor Sets as there are Objects...
                                                                            //NOTE; This WILL limit us to only have ONE texture per Object...

    vk::DescriptorPoolCreateInfo samplerPoolCreateInfo{};
    samplerPoolCreateInfo.setMaxSets(MAX_OBJECTS); 
    samplerPoolCreateInfo.setPoolSizeCount(uint32_t (1));
    samplerPoolCreateInfo.setPPoolSizes(&samplerPoolSize);
    /*// NOTE; While the above code does work (The limit of SamplerDescriptorSets are alot higher than Descriptor Sets for Uniform Buffers)
                It's not the best solution. 
                The Correct way of doing this would be to take advantage of Array Layers and Texture Atlases.
                Right now we are taking up alot of unncessary memory by enabling us to create unncessary Descriptor Sets, 
                We would LIKE to limit the maxSets value to be as low as possible...
    */

    this->samplerDescriptorPool = this->getVkDevice().createDescriptorPool(samplerPoolCreateInfo);
    VulkanDbg::registerVkObjectDbgInfo("DescriptorPool ImageSampler ", vk::ObjectType::eDescriptorPool, reinterpret_cast<uint64_t>(vk::DescriptorPool::CType(this->samplerDescriptorPool)));


    // - CREATE INPUT ATTTACHMENT DESCRIPTOR POOL -
    // Color Attachment Pool Size
    vk::DescriptorPoolSize colorInputPoolSize{};
    colorInputPoolSize.setType(vk::DescriptorType::eInputAttachment);   // Will be used by a Subpass as a Input Attachment... (??) 
    colorInputPoolSize.setDescriptorCount(MAX_FRAMES_IN_FLIGHT);        // One Descriptor per colorBufferImageView

    // Depth attachment Pool Size
    vk::DescriptorPoolSize depthInputPoolSize{};
    depthInputPoolSize.setType(vk::DescriptorType::eInputAttachment);   // Will be used by a Subpass as a Input Attachment... (??) 
    depthInputPoolSize.setDescriptorCount(MAX_FRAMES_IN_FLIGHT);        // One Descriptor per depthBufferImageView

    std::array<vk::DescriptorPoolSize, 2> inputPoolSizes 
    {
        colorInputPoolSize,
        depthInputPoolSize
    };

    // Create Input attachment pool
    vk::DescriptorPoolCreateInfo inputPoolCreateInfo {};
    inputPoolCreateInfo.setMaxSets(this->swapchain.getNumImages());
    inputPoolCreateInfo.setPoolSizeCount(static_cast<uint32_t>(inputPoolSizes.size()));
    inputPoolCreateInfo.setPPoolSizes(inputPoolSizes.data());
    inputPoolCreateInfo.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet); // In order to be able to free this set later (recreate swapchain)

    this->inputDescriptorPool = this->getVkDevice().createDescriptorPool(inputPoolCreateInfo);
    VulkanDbg::registerVkObjectDbgInfo("DescriptorPool InputAttachment ", vk::ObjectType::eDescriptorPool, reinterpret_cast<uint64_t>(vk::DescriptorPool::CType(this->inputDescriptorPool)));
}

void VulkanRenderer::allocateDescriptorSets()
{
    // Resize Descriptor Set; one Descriptor Set per UniformBuffer
    this->descriptorSets.resize(this->viewProjection_uniformBuffer.size()); // Since we have a uniform buffer per images, better use size of swapchainImages!

    // Copy our DescriptorSetLayout so we have one per Image (one per UniformBuffer)
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts(
        this->descriptorSets.size(),
        this->descriptorSetLayout
    );

    // Descriptor Set Allocation Info
    vk::DescriptorSetAllocateInfo setAllocInfo;
    setAllocInfo.setDescriptorPool(this->descriptorPool);                                   // Pool to allocate descriptors (Set?) from   
    setAllocInfo.setDescriptorSetCount(static_cast<uint32_t>(this->descriptorSets.size()));
    setAllocInfo.setPSetLayouts(descriptorSetLayouts.data());                               // Layouts to use to allocate sets (1:1 relationship)

    // Allocate all descriptor sets
    this->descriptorSets = this->getVkDevice().allocateDescriptorSets(setAllocInfo);
}

void VulkanRenderer::createDescriptorSets()
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    // Resize Descriptor Set; one Descriptor Set per UniformBuffer
    this->descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    for(size_t i = 0; i < this->descriptorSets.size(); i++)
    {
        VulkanDbg::registerVkObjectDbgInfo("DescriptorSet["+std::to_string(i)+"]  UniformBuffer", vk::ObjectType::eDescriptorSet, reinterpret_cast<uint64_t>(vk::DescriptorSet::CType(this->descriptorSets[i])));
    }

    // Update all of the Descriptor Set buffer binding
    for(size_t i = 0; i < this->descriptorSets.size(); i++)
    {
        // - VIEW PROJECTION DESCRIPTOR - 
        // Describe the Buffer info and Data offset Info
        vk::DescriptorBufferInfo viewProjection_BufferInfo; 
        viewProjection_BufferInfo.setBuffer(this->viewProjection_uniformBuffer[i]);// Buffer to get the Data from
        viewProjection_BufferInfo.setOffset(0);                                    // Position Of start of Data; 
                                                                                 // Offset from the start (0), since we want to write all data
        viewProjection_BufferInfo.setRange(sizeof(UboViewProjection));             // Size of data ... 

        // Data to describe the connection between Binding and Uniform Buffer
        vk::WriteDescriptorSet viewProjection_setWrite;
        viewProjection_setWrite.setDstSet(this->descriptorSets[i]);              // Descriptor Set to update
        viewProjection_setWrite.setDstBinding(uint32_t (0));                                    // Binding to update (Matches with Binding on Layout/Shader)
        viewProjection_setWrite.setDstArrayElement(uint32_t (0));                                // Index in array we want to update (if we use an array, we do not. thus 0)
        viewProjection_setWrite.setDescriptorType(vk::DescriptorType::eUniformBuffer);// Type of Descriptor
        viewProjection_setWrite.setDescriptorCount(uint32_t (1));                                // Amount of Descriptors to update
        viewProjection_setWrite.setPBufferInfo(&viewProjection_BufferInfo); 

        // List of descriptorSetWrites
        std::vector<vk::WriteDescriptorSet> descriptorSetWrites
        {
            viewProjection_setWrite
        };

        // Update the Descriptor Set with new buffer/binding info
        this->getVkDevice().updateDescriptorSets(
            viewProjection_setWrite,  // Update all Descriptor sets in descripotrSetWrites vector
            nullptr
        );
    }

}

void VulkanRenderer::createInputDescriptorSets()
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    // Resize array to hold Descriptor Set for each frame in flight
    this->inputDescriptorSets.resize(this->swapchain.getNumColorBufferImages());

    // Fill vector of layouts ready for set creation
    std::vector<vk::DescriptorSetLayout> inputSetLayouts(
        this->inputDescriptorSets.size(),
        this->inputSetLayout
    );

    // Input Attachment Descriptor Set Allocation Info
    vk::DescriptorSetAllocateInfo inputSetAllocInfo;
    inputSetAllocInfo.setDescriptorPool(this->inputDescriptorPool);
    inputSetAllocInfo.setDescriptorSetCount(static_cast<uint32_t>(this->inputDescriptorSets.size()));
    inputSetAllocInfo.setPSetLayouts(inputSetLayouts.data());    

    // Allocate Descriptor Sets
    this->inputDescriptorSets = this->getVkDevice().allocateDescriptorSets(inputSetAllocInfo);  

    for(size_t i = 0; i < this->inputDescriptorSets.size(); i++)
    {
        VulkanDbg::registerVkObjectDbgInfo("DescriptorSet[" + std::to_string(i) + "] InputAttachment", vk::ObjectType::eDescriptorSet, reinterpret_cast<uint64_t>(vk::DescriptorSet::CType(this->inputDescriptorSets[i])));

        // Color Attachment Descriptor
        vk::DescriptorImageInfo colorAttachmentDescriptor;
        colorAttachmentDescriptor.setImageLayout(vk::ImageLayout::eReadOnlyOptimal);   // Layout of the Image when it will be Read from! (This is what we've setup the second subpass to expect...)
        colorAttachmentDescriptor.setImageView(this->swapchain.getColorBufferImageView(i));
        colorAttachmentDescriptor.setSampler(VK_NULL_HANDLE);                             // A Subpass Input Attachment can't have a sampler! 

        // Color Attachment Descriptor Write
        vk::WriteDescriptorSet colorWrite;
        colorWrite.setDstSet(this->inputDescriptorSets[i]);
        colorWrite.setDstBinding(uint32_t (0)); 
        colorWrite.setDstArrayElement(uint32_t (0));
        colorWrite.setDescriptorType(vk::DescriptorType::eInputAttachment);
        colorWrite.setDescriptorCount(uint32_t(1));
        colorWrite.setPImageInfo(&colorAttachmentDescriptor);

        
        // Depth Attachment Descriptor
        vk::DescriptorImageInfo depthAttachmentDescriptor;
        depthAttachmentDescriptor.setImageLayout(vk::ImageLayout::eReadOnlyOptimal);   // Layout of the Image when it will be Read from! (This is what we've setup the second subpass to expect...)
        depthAttachmentDescriptor.setImageView(this->swapchain.getDepthBufferImageView(i));
        depthAttachmentDescriptor.setSampler(VK_NULL_HANDLE);                             // A Subpass Input Attachment can't have a sampler! 

        // Depth Attachment Descriptor Write
        vk::WriteDescriptorSet depthWrite;
        depthWrite.setDstSet(this->inputDescriptorSets[i]);
        depthWrite.setDstBinding(uint32_t (1)); 
        depthWrite.setDstArrayElement(uint32_t (0));
        depthWrite.setDescriptorType(vk::DescriptorType::eInputAttachment);
        depthWrite.setDescriptorCount(uint32_t (1));
        depthWrite.setPImageInfo(&depthAttachmentDescriptor);

        // List of the Input Descriptor Sets Writes 
        std::array<vk::WriteDescriptorSet,2> inputDescriptorSetWrites{
            colorWrite,
            depthWrite
        };

        this->getVkDevice().updateDescriptorSets(
            inputDescriptorSetWrites,
            0
        );
    }
    
}

void VulkanRenderer::createCommandPool(vk::CommandPool& commandPool, vk::CommandPoolCreateFlags flags, std::string&& name = "NoName")
{
    vk::CommandPoolCreateInfo commandPoolCreateInfo; 
    commandPoolCreateInfo.setFlags(flags);
    commandPoolCreateInfo.setQueueFamilyIndex(this->queueFamilies.getGraphicsIndex());
    commandPool = this->getVkDevice().createCommandPool(commandPoolCreateInfo);
    VulkanDbg::registerVkObjectDbgInfo(name, vk::ObjectType::eCommandPool, reinterpret_cast<uint64_t>(vk::CommandPool::CType(commandPool)));

    //TODO: automatic trash collection
}

void VulkanRenderer::createCommandBuffer(vk::CommandBuffer& commandBuffer,vk::CommandPool& commandPool, std::string&&  name = "NoName")
{
    vk::CommandBufferAllocateInfo commandBuffferAllocationInfo{};
    commandBuffferAllocationInfo.setCommandPool(commandPool);
    commandBuffferAllocationInfo.setLevel(vk::CommandBufferLevel::ePrimary);
    commandBuffferAllocationInfo.setCommandBufferCount(uint32_t(1));
    
    commandBuffer = this->getVkDevice().allocateCommandBuffers(commandBuffferAllocationInfo)[0];
    VulkanDbg::registerVkObjectDbgInfo(name, vk::ObjectType::eCommandBuffer, reinterpret_cast<uint64_t>(vk::CommandBuffer::CType(commandBuffer)));
    //TODO: automatic trash collection    
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

    frameBuffer = getVkDevice().createFramebuffer(framebufferCreateInfo);
    VulkanDbg::registerVkObjectDbgInfo(name, vk::ObjectType::eFramebuffer, reinterpret_cast<uint64_t>(vk::Framebuffer::CType(frameBuffer)));
}

void VulkanRenderer::updateUniformBuffers()
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    // - REGULAR UNIFORM BUFFER - 
    // - Copy View Projection   - 

    void * data = nullptr;
    
    vmaMapMemory(
        this->vma, 
        this->viewProjection_uniformBufferMemory[this->currentFrame], 
        &data
    );
    memcpy(data, &this->uboViewProjection, sizeof(UboViewProjection));
    vmaUnmapMemory(
        this->vma, 
        this->viewProjection_uniformBufferMemory[this->currentFrame]
    );
}

void VulkanRenderer::updateUBO_camera_Projection()
{
    using namespace vengine_helper::config;
    uboViewProjection.projection  = glm::perspective(                               // View Angle in the y-axis
                            glm::radians(DEF<float>(CAM_FOV)),                               // View Angle in the y-axis
                            (float)this->swapchain.getWidth()/(float)swapchain.getHeight(),         // Setting up the Aspect Ratio
                            DEF<float>(CAM_NP),                                              // The Near Plane
                            DEF<float>(CAM_FP));                                             // The Far Plane
}

void VulkanRenderer::updateUBO_camera_view(glm::vec3 eye, glm::vec3 center, glm::vec3 up)
{
    uboViewProjection.view = glm::lookAt(
                            eye,            // Eye   : Where the Camera is positioned in the world
                            center,         // Target: Point the Camera is looking at
                            up);            // Up    : Up direction 
}

void VulkanRenderer::recordRenderPassCommands_Base(Scene* scene, uint32_t imageIndex)
{
#ifndef VENGINE_NO_PROFILING
    //ZoneScoped; //:NOLINT     
    ZoneTransient(recordRenderPassCommands_zone1,  true); //:NOLINT   
#endif
    // Information about how to befin each Command Buffer...

    vk::CommandBufferBeginInfo bufferBeginInfo;
    bufferBeginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    
    // Information about how to begin a render pass (Only needed for graphical applications)
    vk::RenderPassBeginInfo renderPassBeginInfo;
    renderPassBeginInfo.setRenderPass(this->renderPass_base);                      // Render Pass to Begin
    renderPassBeginInfo.renderArea.setOffset(vk::Offset2D(0, 0));                 // Start of render pass (in pixels...)
    renderPassBeginInfo.renderArea.setExtent(this->swapchain.getVkExtent());          // Size of region to run render pass on (starting at offset)
     
    static const vk::ClearColorValue clear_black(std::array<float,4> {0.F, 0.F, 0.F, 1.F});    
    static const vk::ClearColorValue clear_Plum(std::array<float,4> {221.F/256.0F, 160.F/256.0F, 221.F/256.0F, 1.0F});

    std::array<vk::ClearValue, 3> clearValues = 
    {        // Clear values consists of a VkClearColorValue and a VkClearDepthStencilValue

            vk::ClearValue(                              // of type VkClearColorValue 
                vk::ClearColorValue{clear_black}     // Clear Value for Attachment 0
            ),  
            vk::ClearValue(                              // of type VkClearColorValue 
                vk::ClearColorValue{clear_Plum}     // Clear Value for Attachment 1
            ),            
            vk::ClearValue(                              // Clear Value for Attachment 2
                vk::ClearDepthStencilValue(
                    1.F,    // depth
                    0       // stencil
                )
            )
    };

    renderPassBeginInfo.setPClearValues(clearValues.data());         // List of clear values
    renderPassBeginInfo.setClearValueCount(static_cast<uint32_t>(clearValues.size()));

    // The only changes we do per commandbuffer is to change the swapChainFrameBuffer a renderPass should point to...
    renderPassBeginInfo.setFramebuffer(this->swapchain.getVkFramebuffer(imageIndex));

    vk::SubpassBeginInfoKHR subpassBeginInfo;
    subpassBeginInfo.setContents(vk::SubpassContents::eInline);
    

    vk::CommandBuffer& currentCommandBuffer =
        this->commandBuffers[this->currentFrame];

    // Start recording commands to commandBuffer!
    currentCommandBuffer.begin(bufferBeginInfo);
    {   // Scope for Tracy Vulkan Zone...
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

            // Begin Render Pass!    
            // vk::SubpassContents::eInline; all the render commands themselves will be primary render commands (i.e. will not use secondary commands buffers)
            currentCommandBuffer.beginRenderPass2(renderPassBeginInfo, subpassBeginInfo);

                // Bind Pipeline to be used in render pass
                currentCommandBuffer.bindPipeline(
                    vk::PipelineBindPoint::eGraphics, 
                    this->graphicsPipeline
                );
                
                // For every Mesh we have
                auto tView = scene->getSceneReg().view<Transform, MeshComponent>();
                tView.each([this, currentCommandBuffer](Transform& transform, MeshComponent& meshComponent)
                {
                    auto currModel = modelList[meshComponent.meshID];

                    glm::mat4 modelMatrix = transform.matrix;

                    // auto modelMatrix = currModel.getModelMatrix();

                    // "Push" Constants to given Shader Stage Directly (using no Buffer...)
                    currentCommandBuffer.pushConstants(
                        this->pipelineLayout,
                        vk::ShaderStageFlagBits::eVertex,   // Stage to push the Push Constant to.
                        uint32_t(0),                        // Offset of Push Constants to update; 
                                                            // Offset into the Push Constant Block (if more values are used (??))
                        sizeof(modelMatrix),                // Size of data being pushed
                        &modelMatrix                        // Actual data being pushed (can also be an array)
                    );

                    for(auto& modelPart : currModel.getModelParts())
                    {
                        // -- BINDING VERTEX BUFFERS --
                        //std::array<vk::Buffer,1> vertexBuffer = { currModel.getMesh(k)->getVertexBuffer()};                // Buffers to bind
                        std::array<vk::Buffer,1> vertexBuffer = { modelPart.second.vertexBuffer};                // Buffers to bind
                        std::array<vk::DeviceSize,1> offsets  = {0};                                           // Offsets into buffers being bound
                        currentCommandBuffer.bindVertexBuffers2(
                            uint32_t(0),
                            uint32_t(1),
                            vertexBuffer.data(),
                            offsets.data(),
                            nullptr,        //NOTE: Could also be a pointer to an array of the size in bytes of vertex data bound from pBuffers (vertexBuffer)
                            nullptr         //NOTE: Could also be a pointer to an array of buffer strides
                        );
                        // Bind Mesh Index Buffer; Define the Index Buffer that decides how to draw the Vertex Buffers
                        currentCommandBuffer.bindIndexBuffer(
                            modelPart.second.indexBuffer, 
                            0,
                            vk::IndexType::eUint32);
                      
                        // We're going to bind Two descriptorSets! put them in array...
                        std::array<vk::DescriptorSet,2> descriptorSetGroup{
                            this->descriptorSets[this->currentFrame],                // Use the descriptor set for the Image                            
                            this->samplerDescriptorSets[modelPart.second.textureID]   // Use the Texture which the current mesh has
                        };
                        // Bind Descriptor Sets; this will be the binging for both the Dynamic Uniform Buffers and the non dynamic...
                        currentCommandBuffer.bindDescriptorSets(
                            vk::PipelineBindPoint::eGraphics, // The descriptor set can be used at ANY stage of the Graphics Pipeline
                            this->pipelineLayout,            // The Pipeline Layout that describes how the data will be accessed in our shaders
                            0,                               // Which Set is the first we want to use? We want to use the First set (thus 0)
                            static_cast<uint32_t>(descriptorSetGroup.size()),// How many Descriptor Sets where going to go through? DescriptorSet for View and Projection, and one for Texture
                            descriptorSetGroup.data(),                       // The Descriptor Set to be used (Remember, 1:1 relationship with CommandBuffers/Images)
                            0,                               // Dynamic Offset Count;  we dont Dynamic Uniform Buffers use anymore...
                            nullptr);                        // Dynamic Offset;        We dont use Dynamic Uniform Buffers  anymore...

                        
                        vk::Viewport viewport{};
                        viewport.x = 0.0f;
                        viewport.y = (float) swapchain.getHeight();
                        viewport.width = (float) this->swapchain.getWidth();
                        viewport.height = -((float) swapchain.getHeight());
                        viewport.minDepth = 0.0f;
                        viewport.maxDepth = 1.0f;
                        this->commandBuffers[this->currentFrame].setViewport(0, 1, &viewport);

                        vk::Rect2D scissor{};
                        scissor.offset = vk::Offset2D{0, 0};
                        scissor.extent = this->swapchain.getVkExtent();
                        currentCommandBuffer.setScissor( 0, 1, &scissor);
                        // Execute Pipeline!
                        currentCommandBuffer.drawIndexed(
                            modelPart.second.indexCount,  // Number of vertices to draw (nr of indexes)
                            1,                          // We're drawing only one instance
                            0,                          // Start at index 0
                            0,                          // Vertex offset is 0, i.e. no offset! 
                            0);                         // We Draw Only one Instance, so first will be 0...
                    }                


                });

                // Start Second Subpass
                vk::SubpassEndInfo subpassEndInfo;                
                currentCommandBuffer.nextSubpass2(subpassBeginInfo,subpassEndInfo);

                currentCommandBuffer.bindPipeline(
                    vk::PipelineBindPoint::eGraphics, 
                    this->secondGraphicsPipeline
                );

                currentCommandBuffer.bindDescriptorSets(
                    vk::PipelineBindPoint::eGraphics,
                    this->secondPipelineLayout,
                    uint32_t(0),
                    uint32_t(1),
                    &this->inputDescriptorSets[imageIndex],
                    uint32_t(0),
                    nullptr
                );


                vk::Viewport viewport{};
                viewport.x = 0.0f;
                viewport.y = 0.0f;
                viewport.width = (float) this->swapchain.getWidth();
                viewport.height = (float) swapchain.getHeight();
                viewport.minDepth = 0.0f;
                viewport.maxDepth = 1.0f;
                currentCommandBuffer.setViewport(0, 1, &viewport);

                vk::Rect2D scissor{};
                scissor.offset = vk::Offset2D{0, 0};
                scissor.extent = swapchain.getVkExtent();
                currentCommandBuffer.setScissor( 0, 1, &scissor);

            
                currentCommandBuffer.draw(
                    3,      // We will draw a Triangle, so we only want to draw 3 vertices
                    1,
                    0,
                    0);

            
            // End Render Pass!
            currentCommandBuffer.endRenderPass2(subpassEndInfo);
            
            // Begin second render pass
            vk::RenderPassBeginInfo renderPassBeginInfo{};
            vk::SubpassBeginInfo subpassBeginInfo;
            subpassBeginInfo.setContents(vk::SubpassContents::eInline);
            renderPassBeginInfo.setRenderPass(this->renderPass_imgui);
            renderPassBeginInfo.renderArea.setExtent(this->swapchain.getVkExtent());
            renderPassBeginInfo.renderArea.setOffset(vk::Offset2D(0, 0));
            renderPassBeginInfo.setFramebuffer(this->frameBuffers_imgui[imageIndex]);
            renderPassBeginInfo.setClearValueCount(uint32_t(0));

            currentCommandBuffer.beginRenderPass2(&renderPassBeginInfo, &subpassBeginInfo);

            viewport = vk::Viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = (float)this->swapchain.getWidth();
            viewport.height = (float)swapchain.getHeight();
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            currentCommandBuffer.setViewport(0, 1, &viewport);

            scissor = vk::Rect2D{};
            scissor.offset = vk::Offset2D{ 0, 0 };
            scissor.extent = this->swapchain.getVkExtent();
            currentCommandBuffer.setScissor(0, 1, &scissor);

            ImGui_ImplVulkan_RenderDrawData(
                ImGui::GetDrawData(), 
                currentCommandBuffer
            );

            // End second render pass
            vk::SubpassEndInfo imgui_subpassEndInfo;
            currentCommandBuffer.endRenderPass2(imgui_subpassEndInfo);
        }
        #pragma endregion commandBufferRecording
        
        #ifndef VENGINE_NO_PROFILING
        TracyVkCollect(this->tracyContext[currentImageIndex],
            this->commandBuffers[currentImageIndex]);
        #endif
    }

    // Stop recording to a command buffer
    currentCommandBuffer.end();
}

void VulkanRenderer::recordDynamicRenderingCommands(uint32_t currentImageIndex) 
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    // Information about how to begin each Command Buffer...
    vk::CommandBufferBeginInfo bufferBeginInfo;
    bufferBeginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);    
     
    static const vk::ClearColorValue  clear_black(std::array<float,4> {0.F, 0.F, 0.F, 1.F});    
    static const vk::ClearColorValue  clear_Plum (std::array<float,4> {221.F/256.0F, 160.F/256.0F, 221.F/256.0F, 1.0F});

    vk::RenderingAttachmentInfo color_attachment_info;         
    color_attachment_info.imageView = this->swapchain.getImageView(currentImageIndex);
    color_attachment_info.setImageLayout(vk::ImageLayout::eAttachmentOptimal);
    color_attachment_info.setLoadOp(vk::AttachmentLoadOp::eClear);
    color_attachment_info.setStoreOp(vk::AttachmentStoreOp::eStore);
    color_attachment_info.clearValue = clear_black;

    vk::RenderingAttachmentInfo depth_attachment_info;         
    depth_attachment_info.imageView = this->swapchain.getDepthBufferImageView(currentImageIndex);
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
    renderingInfo.setRenderArea({ vk::Offset2D(0, 0),  this->swapchain.getVkExtent()});
    renderingInfo.setLayerCount(uint32_t (1));    
    renderingInfo.setColorAttachmentCount(uint32_t (1));
    renderingInfo.setPColorAttachments(&color_attachment_info);
    renderingInfo.setPDepthAttachment(&depth_attachment_info);
    renderingInfo.setPStencilAttachment(&depth_attachment_info);

    vk::SubpassBeginInfoKHR subpassBeginInfo;
    subpassBeginInfo.setContents(vk::SubpassContents::eInline);
    
    // Start recording commands to commandBuffer!
    commandBuffers[currentImageIndex].begin(bufferBeginInfo);    
    {   // Scope for Tracy Vulkan Zone...
    #ifndef VENGINE_NO_PROFILING
        TracyVkZone(
            this->tracyContext[currentImageIndex],
            this->commandBuffers[currentImageIndex],"Render Record Commands");
    #endif 

        vengine_helper::insertImageMemoryBarrier(
        createImageBarrierData{
            .cmdBuffer = commandBuffers[currentImageIndex],
            .image = this->swapchain.getImage(currentImageIndex),
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
                .image = this->swapchain.getDepthBufferImage(currentImageIndex),
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
            
            // Begin DynamicRendering!      
            commandBuffers[currentImageIndex].beginRendering(renderingInfo);

                // Bind Pipeline to be used in the DynamicRendering command
                commandBuffers[currentImageIndex].bindPipeline(
                    vk::PipelineBindPoint::eGraphics, 
                    this->graphicsPipeline
                );

                #pragma region commandBufferRecording
                {
                    #ifndef VENGINE_NO_PROFILING
                    ZoneNamedN(loop_all_models, "loop_all_models", true);
                    #endif 
                
                    // For every Mesh we have
                    for(auto & currModel : modelList)
                    {
                        {
                            #ifndef VENGINE_NO_PROFILING
                            ZoneNamedN(loop_per_model, "loop_per_model", true);
                            #endif 

                            auto modelMatrix= currModel.getModelMatrix();

                            // "Push" Constants to given Shader Stage Directly (using no Buffer...)
                            this->commandBuffers[currentImageIndex].pushConstants(
                                this->pipelineLayout,
                                vk::ShaderStageFlagBits::eVertex,   // Stage to push the Push Constant to.
                                uint32_t(0),                        // Offset of Push Constants to update; 
                                                                    // Offset into the Push Constant Block (if more values are used (??))
                                sizeof(modelMatrix),                // Size of data being pushed
                                &modelMatrix                        // Actual data being pushed (can also be an array)
                            );

                            for(auto& modelPart : currModel.getModelParts())
                            {
                                {
                                    #ifndef VENGINE_NO_PROFILING
                                    ZoneNamedN(loop_per_model_part, "loop_per_model_part", true);
                                    #endif 
                                    // -- BINDING VERTEX BUFFERS --                                    
                                    std::array<vk::Buffer,1> vertexBuffer = { modelPart.second.vertexBuffer};                // Buffers to bind
                                    std::array<vk::DeviceSize,1> offsets  = {0};                                           // Offsets into buffers being bound
                                    commandBuffers[currentImageIndex].bindVertexBuffers2(
                                        uint32_t(0),
                                        uint32_t(1),
                                        vertexBuffer.data(),
                                        offsets.data(),
                                        nullptr,        //NOTE: Could also be a pointer to an array of the size in bytes of vertex data bound from pBuffers (vertexBuffer)
                                        nullptr         //NOTE: Could also be a pointer to an array of buffer strides
                                    );
                                    // Bind Mesh Index Buffer; Define the Index Buffer that decides how to draw the Vertex Buffers
                                    commandBuffers[currentImageIndex].bindIndexBuffer(
                                        modelPart.second.indexBuffer, 
                                        0,
                                        vk::IndexType::eUint32);

                                    // We're going to bind Two descriptorSets! put them in array...
                                    std::array<vk::DescriptorSet,2> descriptorSetGroup{
                                        this->descriptorSets[currentImageIndex],                // Use the descriptor set for the Image                            
                                        this->samplerDescriptorSets[ modelPart.second.textureID]   // Use the Texture which the current mesh has
                                    };
                                    // Bind Descriptor Sets; this will be the binging for both the Dynamic Uniform Buffers and the non dynamic...
                                    this->commandBuffers[currentImageIndex].bindDescriptorSets(
                                        vk::PipelineBindPoint::eGraphics, // The descriptor set can be used at ANY stage of the Graphics Pipeline
                                        this->pipelineLayout,            // The Pipeline Layout that describes how the data will be accessed in our shaders
                                        0,                               // Which Set is the first we want to use? We want to use the First set (thus 0)
                                        static_cast<uint32_t>(descriptorSetGroup.size()),// How many Descriptor Sets where going to go through? DescriptorSet for View and Projection, and one for Texture
                                        descriptorSetGroup.data(),                       // The Descriptor Set to be used (Remember, 1:1 relationship with CommandBuffers/Images)
                                        0,                               // Dynamic Offset Count;  we dont Dynamic Uniform Buffers use anymore...
                                        nullptr);                        // Dynamic Offset;        We dont use Dynamic Uniform Buffers  anymore...

                                    // Execute Pipeline!
                                    this->commandBuffers[currentImageIndex].drawIndexed(                            
                                        modelPart.second.indexCount,  // Number of vertices to draw (nr of indexes)
                                        1,                          // We're drawing only one instance
                                        0,                          // Start at index 0
                                        0,                          // Vertex offset is 0, i.e. no offset! 
                                        0);                         // We Draw Only one Instance, so first will be 0...
                                }
                            }
                        }                                               
                    }
                }

            // End DynamicRendering!            
            commandBuffers[currentImageIndex].endRendering();

        #pragma endregion commandBufferRecording
        vengine_helper::insertImageMemoryBarrier(
        createImageBarrierData{
            .cmdBuffer = commandBuffers[currentImageIndex],
            .image = this->swapchain.getImage(currentImageIndex),
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
    // Stop recording to a command buffer
    commandBuffers[currentImageIndex].end();
}


#ifndef VENGINE_NO_PROFILING 
void VulkanRenderer::initTracy()
{
    #ifndef VENGINE_NO_PROFILING
    // Tracy stuff
    //allocateTracyImageMemory();
    vk::DynamicLoader dl; 
    auto pfnvkGetPhysicalDeviceCalibrateableTimeDomainsEXT = dl.getProcAddress<PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT>("vkGetPhysicalDeviceCalibrateableTimeDomainsEXT");

    auto pfnvkGetCalibratedTimestampsEXT = dl.getProcAddress<PFN_vkGetCalibratedTimestampsEXT>("vkGetCalibratedTimestampsEXT");

    // Create Tracy Vulkan Context
    this->tracyContext.resize(this->swapchain.getNumImages());
    for(size_t i = 0 ; i < this->swapchain.getNumImages(); i++){
        
        this->tracyContext[i] = TracyVkContextCalibrated(
            this->physicalDevice.getVkPhysicalDevice(),
            this->getVkDevice(),             
            this->queueFamilies.getGraphicsQueue(),
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

	this->descriptorPool_imgui = this->getVkDevice().createDescriptorPool(pool_info, nullptr);
	

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();    
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    
    io.ConfigWindowsResizeFromEdges = true;
    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
    

    ImGui::StyleColorsDark();
    this->window->initImgui();

    ImGui_ImplVulkan_InitInfo imguiInitInfo {};
    imguiInitInfo.Instance = this->instance.getVkInstance();
    imguiInitInfo.PhysicalDevice = this->physicalDevice.getVkPhysicalDevice();
    imguiInitInfo.Device = this->getVkDevice();
    imguiInitInfo.QueueFamily = this->queueFamilies.getGraphicsIndex();
    imguiInitInfo.Queue = this->queueFamilies.getGraphicsQueue();
    imguiInitInfo.PipelineCache = VK_NULL_HANDLE;  //TODO: Imgui Pipeline Should have its own Cache! 
    imguiInitInfo.DescriptorPool = this->descriptorPool_imgui;
    imguiInitInfo.Subpass = 0; 
    imguiInitInfo.MinImageCount = this->swapchain.getNumMinimumImages();
    imguiInitInfo.ImageCount = this->swapchain.getNumImages();
    imguiInitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT; //TODO: check correctness    
    imguiInitInfo.Allocator = nullptr;    //TODO: Can/should I pass in something VMA related here?
    imguiInitInfo.CheckVkResultFn = checkVkResult;
    ImGui_ImplVulkan_Init(&imguiInitInfo, this->renderPass_imgui);

    std::vector<vk::ImageView> attachment;    
    attachment.resize(1);

    this->createFramebuffer_imgui();

    // Upload imgui font
    this->getVkDevice().resetCommandPool(this->graphicsCommandPool);
    
    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    this->commandBuffers[0].begin(begin_info);
    
    ImGui_ImplVulkan_CreateFontsTexture(this->commandBuffers[0]);

    this->commandBuffers[0].end();
    
    vk::SubmitInfo end_info = {};        
    end_info.commandBufferCount = 1;
    end_info.pCommandBuffers = &this->commandBuffers[0];
    vk::Result result = 
        this->queueFamilies.getGraphicsQueue().submit(
            1, 
            &end_info, 
            VK_NULL_HANDLE
        );
    if(result != vk::Result::eSuccess)
    {
        Log::error("Failed to submit imgui fonts to graphics queue...");
    }

    this->device.waitIdle();

    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void VulkanRenderer::createFramebuffer_imgui()
{
    std::vector<vk::ImageView> attachment;    
    attachment.resize(1);
    this->frameBuffers_imgui.resize(this->swapchain.getNumImages());
    for(size_t i = 0; i < this->frameBuffers_imgui.size(); i++)
    {        
        attachment[0] = this->swapchain.getImageView(i); 
        createFrameBuffer(
            this->frameBuffers_imgui[i], 
            attachment, 
            this->renderPass_imgui, 
            this->swapchain.getVkExtent(), 
            std::string("frameBuffers_imgui["+std::to_string(i)+"]")
        );
    }
}

void VulkanRenderer::cleanupFramebuffer_imgui()
{
    for (auto framebuffer: this->frameBuffers_imgui) 
    {
        this->getVkDevice().destroyFramebuffer(framebuffer);        
    }
}
