#include "VulkanRenderer.hpp"
#include "Utilities.hpp"
#include "assimp/Importer.hpp"
#include "vulkan/VulkanValidation.hpp"
#include "vulkan/VulkanDbg.hpp"
#include "../dev/defs.hpp"
#include "../dev/tracyHelper.hpp"
#include "../ResourceManagement/Configurator.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <utility>
#include <fstream>
#include <set>
#include <vector>
#include <limits>               
#include <algorithm>            
#include "stb_image.h"
#include "Texture.hpp"
#include "Buffer.hpp"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "backends/imgui_impl_vulkan.h"

#include "../application/Input.hpp"
#include "../application/Scene.hpp"
#include "../components/MeshComponent.hpp"
#include "../dev/Log.hpp"
#include "../ResourceManagement/ResourceManager.hpp"
#include "../ResourceManagement/loaders/MeshLoader.hpp"
#include "../ResourceManagement/loaders/TextureLoader.hpp"

static void checkVkResult(VkResult err)
{
    if (err == 0)
        return;

    Log::error("Vulkan error from imgui: " + std::to_string(err));
}

void VulkanRenderer::initResourceManager()
{
    this->resourceMan->init(&this->vma,
        &this->physicalDevice.getVkPhysicalDevice(),
        &this->device,
        &this->queueFamilies.getGraphicsQueue(),
        &this->graphicsCommandPool,
        this); /// TODO:  <-- REMOVE THIS, temporary used before making createTexture part of resourceManager...

    
    
}

using namespace vengine_helper::config;
int VulkanRenderer::init(Window* window, std::string&& windowName, ResourceManager* resourceMan)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif        

    this->resourceMan = resourceMan;
    this->window = window;

    try 
    {
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

        VulkanDbg::registerVkObjectDbgInfo("Graphics Queue", vk::ObjectType::eQueue, reinterpret_cast<uint64_t>(vk::Queue::CType(this->queueFamilies.getGraphicsQueue())));
        VulkanDbg::registerVkObjectDbgInfo("Presentation Queue", vk::ObjectType::eQueue, reinterpret_cast<uint64_t>(vk::Queue::CType(this->queueFamilies.getPresentQueue()))); // TODO: might be problematic.. since it can be same as Graphics Queue
             
        VulkanDbg::registerVkObjectDbgInfo("Surface",vk::ObjectType::eSurfaceKHR, reinterpret_cast<uint64_t>(vk::SurfaceKHR::CType(this->surface)));                
        VulkanDbg::registerVkObjectDbgInfo("PhysicalDevice",vk::ObjectType::ePhysicalDevice, reinterpret_cast<uint64_t>(vk::PhysicalDevice::CType(this->physicalDevice.getVkPhysicalDevice())));                
        VulkanDbg::registerVkObjectDbgInfo("Logical Device",vk::ObjectType::eDevice, reinterpret_cast<uint64_t>(vk::Device::CType(this->getVkDevice())));

        VmaAllocatorCreateInfo vmaAllocatorCreateInfo{};
        vmaAllocatorCreateInfo.instance = this->instance.getVkInstance();
        vmaAllocatorCreateInfo.physicalDevice = this->physicalDevice.getVkPhysicalDevice();
        vmaAllocatorCreateInfo.device = this->getVkDevice();
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

        this->createRenderPassBase();
        this->createRenderPassImgui();
        this->createDescriptorSetLayout();
        
        this->createPushConstantRange();

        this->pipelineLayout.createPipelineLayout(
            this->device,
            this->descriptorSetLayout,
            this->samplerDescriptorSetLayout,
            this->pushConstantRange
        );
        this->pipeline.createPipeline(
            this->device, 
            this->pipelineLayout,
            this->renderPassBase
        );
        
        this->swapchain.createFramebuffers(this->renderPassBase);
        this->createCommandPool();

        this->createCommandBuffers();

        this->createTextureSampler();
        
        this->viewProjectionUB.createUniformBuffer(
            this->device,
            this->vma,
            sizeof(UboViewProjection),
            MAX_FRAMES_IN_FLIGHT
        );

        this->createDescriptorPool();

        this->allocateDescriptorSets();
        this->createDescriptorSets();
        
        this->createSynchronisation();        

        this->updateUboProjection(); //TODO: Allow for more cameras! 
        this->updateUboView(
            glm::vec3(DEF<float>(CAM_EYE_X),DEF<float>(CAM_EYE_Y),DEF<float>(CAM_EYE_Z)),
            glm::vec3(DEF<float>(CAM_TARGET_X),DEF<float>(CAM_TARGET_Y), DEF<float>(CAM_TARGET_Z)));

#ifndef VENGINE_NO_PROFILING
        this->initTracy();
#endif
        this->initImgui();

        this->initResourceManager();

        // Setup Fallback Texture: Let first Texture be default if no other texture is found.
        this->resourceMan->addTexture("missing_texture.png");

    }
    catch(std::runtime_error &e)
    {
        std::cout << "ERROR: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


void VulkanRenderer::generateVmaDump()
{
    Log::error("generateVmaDump() is causing a memory leak at the moment :(");

    char* vma_dump;
    vmaBuildStatsString(this->vma, &vma_dump, VK_TRUE);
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
    tracy::GetProfiler().RequestShutdown();  
#endif
    
    //Wait until no actions is run on device...
    this->device.waitIdle(); // Dont destroy semaphores before they are done

    this->resourceMan->cleanup(); //TODO: Rewrite cleanup, "sartCleanup"
    
    ImGui_ImplVulkan_Shutdown();
    this->window->shutdownImgui();
    ImGui::DestroyContext();

    this->cleanupFramebufferImgui();

    this->getVkDevice().destroyRenderPass(this->renderPassImgui);
    this->getVkDevice().destroyDescriptorPool(this->descriptorPoolImgui);
    
#ifndef VENGINE_NO_PROFILING
    CustomFree(this->tracyImage);

    for(auto &tracy_context : this->tracyContext)
    {
        TracyVkDestroy(tracy_context);
    }
#endif

    //TODO: Check if this should be removed... Probably should
    //this->getVkDevice().destroyDescriptorPool(this->inputDescriptorPool);
    //this->getVkDevice().destroyDescriptorSetLayout(this->inputSetLayout);

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

    this->viewProjectionUB.cleanup();

    for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        this->getVkDevice().destroySemaphore(this->renderFinished[i]);
        this->getVkDevice().destroySemaphore(this->imageAvailable[i]);
        this->getVkDevice().destroyFence(this->drawFences[i]);        
    }

    this->getVkDevice().destroyCommandPool(this->graphicsCommandPool);
    
    this->getVkDevice().destroyPipelineCache(this->graphics_pipelineCache);

    this->pipeline.cleanup();
    this->pipelineLayout.cleanup();

    this->getVkDevice().destroyRenderPass(this->renderPassBase);

    this->swapchain.cleanup();

    this->instance.destroy(this->surface); //NOTE: No warnings/errors if we run this line... Is it useless? Mayber gets destroyed by SDL?
    
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
            uint32_t(1),                          // number of Fences to wait on
            &this->drawFences[this->currentFrame],// Which Fences to wait on
            waitForAllFences,                     // should we wait for all Fences or not?              
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
            std::numeric_limits<uint64_t>::max(),     // How long to wait before the Image is retrieved, crash if reached. 
                                                      // We dont want to use a timeout, so we make it as big as possible.
            this->imageAvailable[this->currentFrame], // The Semaphore to signal, when it's available to be used!
            VK_NULL_HANDLE                            // The Fence to signal, when it's available to be used...(??)
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
    this->viewProjectionUB.update(
        (void*) &this->uboViewProjection, 
        this->currentFrame
    );

    // ReRecord the current CommandBuffer! In order to update any Push Constants
    recordRenderPassCommandsBase(scene, imageIndex);
    
    // Submit to graphics queue
    {
        #ifndef VENGINE_NO_PROFILING
        ZoneNamedN(draw_zone3, "Wait for Semaphore", true); //:NOLINT   
        #endif 

        // -- Submit command buffer to Render -- 
        //2. Submit command buffer to queue for execution, making sure it waits for the image to be signalled as 
        //   available before drawing and signals when it has finished renedering. 
        
        std::array<vk::PipelineStageFlags2, 1> waitStages = {   // Definies What stages the Semaphore have to wait on.        
            vk::PipelineStageFlagBits2::eColorAttachmentOutput  // Stage: Start drawing to the Framebuffer...
        };
        
        vk::SemaphoreSubmitInfo wait_semaphoreSubmitInfo;
        wait_semaphoreSubmitInfo.setSemaphore(this->imageAvailable[this->currentFrame]);
        wait_semaphoreSubmitInfo.setStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput);         
        wait_semaphoreSubmitInfo.setDeviceIndex(uint32_t(1));                    // 0: sets all devices in group 1 to valid... bad or good?

        vk::SemaphoreSubmitInfo signal_semaphoreSubmitInfo;
        signal_semaphoreSubmitInfo.setSemaphore(this->renderFinished[this->currentFrame]);
        signal_semaphoreSubmitInfo.setStageMask(vk::PipelineStageFlags2());      // Stages to check semaphores at    

        std::vector<vk::CommandBufferSubmitInfo> commandBufferSubmitInfos{
            vk::CommandBufferSubmitInfo{this->commandBuffers[this->currentFrame]}
        };        
        
        vk::SubmitInfo2 submitInfo {};      
        submitInfo.setWaitSemaphoreInfoCount(uint32_t(1));
        // !!!submitInfo.setWaitSemaphoreInfos(const vk::ArrayProxyNoTemporaries<const vk::SemaphoreSubmitInfo> &waitSemaphoreInfos_)
        submitInfo.setPWaitSemaphoreInfos(&wait_semaphoreSubmitInfo);       // Pointer to the semaphore to wait on.
        submitInfo.setCommandBufferInfoCount(commandBufferSubmitInfos.size()); 
        submitInfo.setPCommandBufferInfos(commandBufferSubmitInfos.data()); // Pointer to the CommandBuffer to execute
        submitInfo.setSignalSemaphoreInfoCount(uint32_t(1));
        submitInfo.setPSignalSemaphoreInfos(&signal_semaphoreSubmitInfo);   // Semaphore that will be signaled when CommandBuffer is finished

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
        presentInfo.setPSwapchains(&this->swapchain.getVkSwapchain());              // Swapchain to present the image to
        presentInfo.setPImageIndices(&imageIndex);                                  // Index of images in swapchains to present                

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
        meshComponent.meshID = this->resourceMan->addMesh("ghost.obj");
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

    // Using SDL to create WindowSurface, to make it cross platform
    // Creates a surface create info struct configured for how SDL handles windows.    
    this->window->createVulkanSurface(this->instance, this->surface);
}

void VulkanRenderer::recreateSwapchain(Camera* camera)
{
    this->device.waitIdle();
    
    cleanupFramebufferImgui();

    this->swapchain.recreateSwapchain(this->renderPassBase);
    createFramebufferImgui();

    this->createDescriptorSets();

    ImGui_ImplVulkan_SetMinImageCount(this->swapchain.getNumMinimumImages());

    // Take new aspect ratio into account for the camera
    camera->aspectRatio = (float) this->swapchain.getWidth() / (float)swapchain.getHeight();
    camera->projection = glm::perspective(camera->fov, camera->aspectRatio, 0.1f, 100.0f);
    camera->invProjection = glm::inverse(camera->projection);
}

void VulkanRenderer::cleanupRenderPassImgui()
{
    this->getVkDevice().destroyRenderPass(this->renderPassImgui);
}

void VulkanRenderer::cleanupRenderPassBase()
{
    this->getVkDevice().destroyRenderPass(this->renderPassBase);
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
    imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);      // The Image Layout when it is in use
    imageInfo.setImageView(textureImage);                                   // Image to be bind to set
    imageInfo.setSampler(this->textureSampler);                             // the Sampler to use for this Descriptor Set

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

VulkanRenderer::VulkanRenderer()
    : resourceMan(nullptr), window(nullptr)
{
    loadConfIntoMemory();
}

void VulkanRenderer::createRenderPassBase() 
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    // Array of our subPasses        
    std::array<vk::SubpassDescription2, 1> subPasses{};

    // Color Attachment
    vk::AttachmentDescription2 colorAttachment {};
    colorAttachment.setFormat(this->swapchain.getVkFormat());
    colorAttachment.setSamples(vk::SampleCountFlagBits::e1);
    colorAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);        // When we start the renderpass, first thing to do is to clear since there is no values in it yet
    colorAttachment.setStoreOp(vk::AttachmentStoreOp::eStore);      // How to store it after the RenderPass; We dont care! But we do care what happens after the first SubPass! (not handled here)
    colorAttachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
    colorAttachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
    colorAttachment.setInitialLayout(vk::ImageLayout::eUndefined);            // We dont care what the image layout is when we start. But we do care about what layout it is when it enter the first SubPass! (not handled here)
    colorAttachment.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal); //(!!) Should be the same value as it was after the subpass finishes (??)

    // Depth Attatchment
    vk::AttachmentDescription2 depthAttachment{};
    depthAttachment.setFormat(this->swapchain.getVkDepthFormat());
    depthAttachment.setSamples(vk::SampleCountFlagBits::e1);
    depthAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);                         // Clear Buffer Whenever we try to load data into (i.e. clear before use it!)
    depthAttachment.setStoreOp(vk::AttachmentStoreOp::eDontCare);                    // Whenever it's used, we dont care what happens with the data... (we dont present it or anything)
    depthAttachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);               // Even though the Stencil i present, we dont plan to use it. so we dont care    
    depthAttachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);             // Even though the Stencil i present, we dont plan to use it. so we dont care
    depthAttachment.setInitialLayout(vk::ImageLayout::eUndefined);                   // We don't care how the image layout is initially, so let it be undefined
    depthAttachment.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal); // Final layout should be Optimal for Depth Stencil attachment!

    // Color attachment reference
    vk::AttachmentReference2 colorAttachmentReference {};    
    colorAttachmentReference.setAttachment(uint32_t(0));                          // Match the number/ID of the Attachment to the index of the FrameBuffer!
    colorAttachmentReference.setLayout(vk::ImageLayout::eColorAttachmentOptimal); // The Layout the Subpass must be in! 

    // Depth attachment reference
    vk::AttachmentReference2 depthAttachmentReference {};
    depthAttachmentReference.setAttachment(uint32_t(1)); 
    depthAttachmentReference.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal); // The layout the subpass must be in! Should be same as 'final layout'(??)

    // Setup Subpass 0
    subPasses[0].setPipelineBindPoint(vk::PipelineBindPoint::eGraphics); 
    subPasses[0].setColorAttachmentCount(uint32_t(1)); 
    subPasses[0].setPColorAttachments(&colorAttachmentReference); 
    subPasses[0].setPDepthStencilAttachment(&depthAttachmentReference); 

    // Override the first implicit subpass
    std::array<vk::SubpassDependency2, 1> subpassDependencies{};
    subpassDependencies[0].setSrcSubpass(VK_SUBPASS_EXTERNAL);
    subpassDependencies[0].setDstSubpass(0);
    subpassDependencies[0].setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests);
    subpassDependencies[0].setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests);
    subpassDependencies[0].setSrcAccessMask(vk::AccessFlagBits::eNone);
    subpassDependencies[0].setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite);
    subpassDependencies[0].setDependencyFlags(vk::DependencyFlagBits::eByRegion);

    // Vector with the Attatchments
    std::array<vk::AttachmentDescription2, 2> attachments
    {
        colorAttachment,
        depthAttachment
    };

    //Create info for render pass
    vk::RenderPassCreateInfo2 renderPassCreateInfo;
    renderPassCreateInfo.setAttachmentCount(static_cast<uint32_t>(attachments.size()));
    renderPassCreateInfo.setPAttachments(attachments.data());
    renderPassCreateInfo.setSubpassCount(static_cast<uint32_t>(subPasses.size()));
    renderPassCreateInfo.setPSubpasses(subPasses.data());
    renderPassCreateInfo.setDependencyCount(static_cast<uint32_t> (subpassDependencies.size()));
    renderPassCreateInfo.setPDependencies(subpassDependencies.data());

    this->renderPassBase = this->getVkDevice().createRenderPass2(renderPassCreateInfo);

    VulkanDbg::registerVkObjectDbgInfo("The RenderPass", vk::ObjectType::eRenderPass, reinterpret_cast<uint64_t>(vk::RenderPass::CType(this->renderPassBase)));
}

void VulkanRenderer::createRenderPassImgui()
{
    vk::AttachmentDescription2 attachment{};
    attachment.setFormat(this->swapchain.getVkFormat());
    attachment.setSamples(vk::SampleCountFlagBits::e1);
    attachment.setLoadOp(vk::AttachmentLoadOp::eDontCare);
    attachment.setStoreOp(vk::AttachmentStoreOp::eStore);
    attachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
    attachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
    attachment.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal);
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
    this->renderPassImgui = this->getVkDevice().createRenderPass2(renderpassCreateinfo);
}

void VulkanRenderer::createDescriptorSetLayout()
{    
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    // - CREATE UNIFORM VALUES DESCRIPTOR SET LAYOUT -

    // UboViewProjection binding Info
    vk::DescriptorSetLayoutBinding vpLayoutBinding;
    vpLayoutBinding.setBinding(uint32_t (0));                                   // Describes which Binding Point in the shaders this layout is being bound to
    vpLayoutBinding.setDescriptorType(vk::DescriptorType::eUniformBuffer);      // Type of descriptor (Uniform, Dynamic uniform, image Sampler, etc.)
    vpLayoutBinding.setDescriptorCount(uint32_t(1));                            // Amount of actual descriptors we're binding, where just binding one; our MVP struct
    vpLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eVertex);            // What Shader Stage we want to bind our Descriptor set to
    vpLayoutBinding.setPImmutableSamplers(nullptr);//vknullhandle??             // Used by Textures; whether or not the Sampler should be Immutable

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
    samplerLayoutBinding.setBinding(uint32_t (0));                                  // This can be 0 too, as it will be for a different Descriptor Set, Descriptor set 1 (previous was Descriptor Set 0)! 
    samplerLayoutBinding.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
    samplerLayoutBinding.setDescriptorCount(uint32_t (1));               
    samplerLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eFragment);         // The Stage the descriptor layout will pass to will be the Fragment Shader
    samplerLayoutBinding.setPImmutableSamplers(nullptr);

    // Create a Descriptor Set Layout with given bindings for texture
    vk::DescriptorSetLayoutCreateInfo textureLayoutCreateInfo;
    textureLayoutCreateInfo.setBindingCount(uint32_t (1));
    textureLayoutCreateInfo.setPBindings(&samplerLayoutBinding);

    // create Descriptor Set Layout
    this->samplerDescriptorSetLayout = this->getVkDevice().createDescriptorSetLayout(textureLayoutCreateInfo);
    VulkanDbg::registerVkObjectDbgInfo("DescriptorSetLayout SamplerTexture", vk::ObjectType::eDescriptorSetLayout, reinterpret_cast<uint64_t>(vk::DescriptorSetLayout::CType(this->samplerDescriptorSetLayout)));
}

void VulkanRenderer::createPushConstantRange()
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    // Define the Push Constants values
    this->pushConstantRange.setStageFlags(vk::ShaderStageFlagBits::eVertex);    // Push Constant should be available in the Vertex Shader!
    this->pushConstantRange.setOffset(uint32_t (0));                            // Offset into the given data that our Push Constant value is (??)
    this->pushConstantRange.setSize(sizeof(ModelMatrix));                       // Size of the Data being passed
}

void VulkanRenderer::createCommandPool()
 {
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    vk::CommandPoolCreateInfo poolInfo = {};
    poolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);  // Enables us to reset our CommandBuffers 
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
    samplerCreateInfo.setMagFilter(vk::Filter::eLinear);                     // How the sampler will sample from a texture when it's getting closer
    samplerCreateInfo.setMinFilter(vk::Filter::eLinear);                     // How the sampler will sample from a texture when it's getting further away
    samplerCreateInfo.setAddressModeU(vk::SamplerAddressMode::eRepeat);      // How the texture will be Wrapped in U (x) direction
    samplerCreateInfo.setAddressModeV(vk::SamplerAddressMode::eRepeat);      // How the texture will be Wrapped in V (y) direction
    samplerCreateInfo.setAddressModeW(vk::SamplerAddressMode::eRepeat);      // How the texture will be Wrapped in W (z) direction
    samplerCreateInfo.setBorderColor(vk::BorderColor::eIntOpaqueBlack);      // Color of what is around the texture (in case of Repeat, it wont be used)
    samplerCreateInfo.setUnnormalizedCoordinates(VK_FALSE);                  // We want to used Normalised Coordinates (between 0 and 1), so unnormalized coordinates must be false... 
    samplerCreateInfo.setMipmapMode(vk::SamplerMipmapMode::eLinear);         // How the mipmap mode will switch between the mipmap images (interpolate between images), (we dont use it, but we set it up)
    samplerCreateInfo.setMipLodBias(0.F);                                    // Level of detail bias for mip level...
    samplerCreateInfo.setMinLod(0.F);                                        // Minimum level of Detail to pick mip level
    samplerCreateInfo.setMaxLod(VK_LOD_CLAMP_NONE);                          // Maxiumum level of Detail to pick mip level
    samplerCreateInfo.setAnisotropyEnable(VK_TRUE);                          // Enable Anisotropy; take into account the angle of a surface is being viewed from and decide details based on that (??)
    //samplerCreateInfo.setAnisotropyEnable(VK_FALSE);                       // Disable Anisotropy; Cause Performance Issues according to validation... 
                                                                             // TODO: Check how anisotrophy can be used without causing validation errors... ? 
    samplerCreateInfo.setMaxAnisotropy(DEF<float>(SAMPL_MAX_ANISOSTROPY));   // Level of Anisotropy; 16 is a common option in the settings for alot of Games 

    this->textureSampler = this->getVkDevice().createSampler(samplerCreateInfo);
    VulkanDbg::registerVkObjectDbgInfo("Texture Sampler", vk::ObjectType::eSampler, reinterpret_cast<uint64_t>(vk::Sampler::CType(this->textureSampler)));
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
    viewProjection_poolSize.setType(vk::DescriptorType::eUniformBuffer);    // Descriptors in Set will be of Type Uniform Buffer    
    viewProjection_poolSize.setDescriptorCount(MAX_FRAMES_IN_FLIGHT);       // How many Descriptors we want, we want One uniformBuffer so we its only the size of our uniformBuffer

    std::vector<vk::DescriptorPoolSize> descriptorPoolSizes
    {
        viewProjection_poolSize
    };

    // Data to create Descriptor Pool
    vk::DescriptorPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.setMaxSets(MAX_FRAMES_IN_FLIGHT);             // Max Nr Of descriptor Sets that can be created from the pool, 
                                                                                        // Same as the number of buffers / images we have. 
    poolCreateInfo.setPoolSizeCount(static_cast<uint32_t>(descriptorPoolSizes.size())); // Based on how many pools we have in our descriptorPoolSizes
    poolCreateInfo.setPPoolSizes(descriptorPoolSizes.data());                           // PoolSizes to create the Descriptor Pool with

    this->descriptorPool = this->getVkDevice().createDescriptorPool(poolCreateInfo);
    VulkanDbg::registerVkObjectDbgInfo("DescriptorPool UniformBuffer ", vk::ObjectType::eDescriptorPool, reinterpret_cast<uint64_t>(vk::DescriptorPool::CType(this->descriptorPool)));
        
    // - CRTEATE SAMPLER DESCRIPTOR POOL -
    // Texture Sampler Pool
    vk::DescriptorPoolSize samplerPoolSize{};
    samplerPoolSize.setType(vk::DescriptorType::eCombinedImageSampler); // This descriptor pool will have descriptors for Image and Sampler combined    
                                                                        // NOTE; Should be treated as seperate Concepts! but this will be enough...
    samplerPoolSize.setDescriptorCount(MAX_OBJECTS);                    // There will be as many Descriptor Sets as there are Objects...
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
}

void VulkanRenderer::allocateDescriptorSets()
{
    // Resize Descriptor Set; one Descriptor Set per UniformBuffer
    this->descriptorSets.resize(this->viewProjectionUB.getNumBuffers()); // Since we have a uniform buffer per images, better use size of swapchainImages!

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

    // Update all of the Descriptor Set buffer binding
    for(size_t i = 0; i < this->descriptorSets.size(); i++)
    {
        // - VIEW PROJECTION DESCRIPTOR - 
        // Describe the Buffer info and Data offset Info
        vk::DescriptorBufferInfo viewProjectionBufferInfo; 
        viewProjectionBufferInfo.setBuffer(this->viewProjectionUB.getBuffer(i)); // Buffer to get the Data from
        viewProjectionBufferInfo.setOffset(0);
        viewProjectionBufferInfo.setRange((vk::DeviceSize) this->viewProjectionUB.getBufferSize());

        // Data to describe the connection between Binding and Uniform Buffer
        vk::WriteDescriptorSet viewProjectionWriteSet;
        viewProjectionWriteSet.setDstSet(this->descriptorSets[i]);                   // Descriptor Set to update
        viewProjectionWriteSet.setDstBinding(uint32_t(0));                           // Binding to update (Matches with Binding on Layout/Shader)
        viewProjectionWriteSet.setDstArrayElement(uint32_t(0));                      // Index in array we want to update (if we use an array, we do not. thus 0)
        viewProjectionWriteSet.setDescriptorType(vk::DescriptorType::eUniformBuffer);// Type of Descriptor
        viewProjectionWriteSet.setDescriptorCount(uint32_t(1));                      // Amount of Descriptors to update
        viewProjectionWriteSet.setPBufferInfo(&viewProjectionBufferInfo);

        // List of descriptorSetWrites
        std::vector<vk::WriteDescriptorSet> writeDescriptorSets
        {
            viewProjectionWriteSet
        };

        // Update the Descriptor Set with new buffer/binding info
        this->getVkDevice().updateDescriptorSets(
            writeDescriptorSets,  // Update all Descriptor sets in writeDescriptorSets vector
            nullptr
        );

        VulkanDbg::registerVkObjectDbgInfo("DescriptorSet[" + std::to_string(i) + "]  UniformBuffer", vk::ObjectType::eDescriptorSet, reinterpret_cast<uint64_t>(vk::DescriptorSet::CType(this->descriptorSets[i])));
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

void VulkanRenderer::createFramebuffer(
    vk::Framebuffer& frameBuffer,
    std::vector<vk::ImageView>& attachments,
    vk::RenderPass& renderPass, 
    vk::Extent2D& extent, 
    std::string&& name = "NoName")
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

void VulkanRenderer::updateUboProjection()
{
    using namespace vengine_helper::config;
    uboViewProjection.projection  = glm::perspective(                                        // View Angle in the y-axis
                            glm::radians(DEF<float>(CAM_FOV)),                               // View Angle in the y-axis
                            (float)this->swapchain.getWidth()/(float)swapchain.getHeight(),  // Setting up the Aspect Ratio
                            DEF<float>(CAM_NP),                                              // The Near Plane
                            DEF<float>(CAM_FP));                                             // The Far Plane
}

void VulkanRenderer::updateUboView(glm::vec3 eye, glm::vec3 center, glm::vec3 up)
{
    uboViewProjection.view = glm::lookAt(
                            eye,            // Eye   : Where the Camera is positioned in the world
                            center,         // Target: Point the Camera is looking at
                            up);            // Up    : Up direction 
}

void VulkanRenderer::recordRenderPassCommandsBase(Scene* scene, uint32_t imageIndex)
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
    renderPassBeginInfo.setRenderPass(this->renderPassBase);                      // Render Pass to Begin
    renderPassBeginInfo.renderArea.setOffset(vk::Offset2D(0, 0));                 // Start of render pass (in pixels...)
    renderPassBeginInfo.renderArea.setExtent(this->swapchain.getVkExtent());      // Size of region to run render pass on (starting at offset)
     
    static const vk::ClearColorValue clear_black(std::array<float,4> {0.F, 0.F, 0.F, 1.F});    
    static const vk::ClearColorValue clear_Plum(std::array<float,4> {221.F/256.0F, 160.F/256.0F, 221.F/256.0F, 1.0F});

    std::array<vk::ClearValue, 2> clearValues = 
    {
            vk::ClearValue(                         // of type VkClearColorValue 
                vk::ClearColorValue{clear_Plum}     // Clear Value for Attachment 0
            ),  
            vk::ClearValue(                         // Clear Value for Attachment 1
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
            this->tracyContext[imageIndex],
            this->commandBuffers[imageIndex],"Render Record Commands");
        #endif
        {
        #ifndef VENGINE_NO_PROFILING        
        ZoneTransient(recordRenderPassCommands_zone2,  true); //:NOLINT   
        #endif
        
        #pragma region commandBufferRecording

            // Begin Render Pass!    
            // vk::SubpassContents::eInline; all the render commands themselves will be primary render commands (i.e. will not use secondary commands buffers)
            currentCommandBuffer.beginRenderPass2(renderPassBeginInfo, subpassBeginInfo);

                vk::Viewport viewport{};
                viewport.x = 0.0f;
                viewport.y = (float)swapchain.getHeight();
                viewport.width = (float)this->swapchain.getWidth();
                viewport.height = -((float)swapchain.getHeight());
                viewport.minDepth = 0.0f;
                viewport.maxDepth = 1.0f;
                currentCommandBuffer.setViewport(0, 1, &viewport);

                vk::Rect2D scissor{};
                scissor.offset = vk::Offset2D{ 0, 0 };
                scissor.extent = this->swapchain.getVkExtent();
                currentCommandBuffer.setScissor(0, 1, &scissor);

                // Bind Pipeline to be used in render pass
                currentCommandBuffer.bindPipeline(
                    vk::PipelineBindPoint::eGraphics,
                    this->pipeline.getVkPipeline()
                );
                
                // For every Mesh we have
                auto tView = scene->getSceneReg().view<Transform, MeshComponent>();
                tView.each([this, currentCommandBuffer](const Transform& transform, const MeshComponent& meshComponent)
                {
                    auto& currModel = this->resourceMan->getMesh(meshComponent.meshID);                    

                    const glm::mat4& modelMatrix = transform.matrix;

                    // auto modelMatrix = currModel.getModelMatrix();

                    // "Push" Constants to given Shader Stage Directly (using no Buffer...)
                    currentCommandBuffer.pushConstants(
                        this->pipelineLayout.getVkPipelineLayout(),
                        vk::ShaderStageFlagBits::eVertex,   // Stage to push the Push Constant to.
                        uint32_t(0),                        // Offset of Push Constants to update; 
                                                            // Offset into the Push Constant Block (if more values are used (??))
                        sizeof(modelMatrix),                // Size of data being pushed
                        &modelMatrix                        // Actual data being pushed (can also be an array)
                    );

                    // -- BINDING VERTEX BUFFERS --                    
                    std::array<vk::Buffer,1> vertexBuffer = { currModel.getVertexBuffer()}; // Buffers to bind
                    std::array<vk::DeviceSize,1> offsets  = {0};                            // Offsets into buffers being bound
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
                        currModel.getIndexBuffer(), 
                        0,
                        vk::IndexType::eUint32);
                    
                    for(auto& submesh : currModel.getSubmeshData()){                        
                        // We're going to bind Two descriptorSets! put them in array...
                        std::array<vk::DescriptorSet,2> descriptorSetGroup{
                            this->descriptorSets[this->currentFrame],                // Use the descriptor set for the Image                  
                            this->samplerDescriptorSets[submesh.materialIndex]       // Use the Texture which the current mesh has
                        };
                        // Bind Descriptor Sets; this will be the binging for both the Dynamic Uniform Buffers and the non dynamic...
                        currentCommandBuffer.bindDescriptorSets( //TODO: Check If correct after merge (alt. imageIndex)
                            vk::PipelineBindPoint::eGraphics,    // The descriptor set can be used at ANY stage of the Graphics Pipeline
                            this->pipelineLayout.getVkPipelineLayout(),      // The Pipeline Layout that describes how the data will be accessed in our shaders
                            0,                                               // Which Set is the first we want to use? We want to use the First set (thus 0)
                            static_cast<uint32_t>(descriptorSetGroup.size()),// How many Descriptor Sets where going to go through? DescriptorSet for View and Projection, and one for Texture
                            descriptorSetGroup.data(),                       // The Descriptor Set to be used (Remember, 1:1 relationship with CommandBuffers/Images)
                            0,                               // Dynamic Offset Count;  we dont Dynamic Uniform Buffers use anymore...
                            nullptr);                        // Dynamic Offset;        We dont use Dynamic Uniform Buffers  anymore...
                        
                        // Execute Pipeline!
                        currentCommandBuffer.drawIndexed( //TODO: Check if correct after merge (alt imageIndex)
                            submesh.numIndicies,        // Number of vertices to draw (nr of indexes)
                            1,                          // We're drawing only one instance
                            submesh.startIndex,         
                            0,                          // Vertex offset is 0, i.e. no offset! 
                            0);                         // We Draw Only one Instance, so first will be 0...
                    }

                });

            // End Render Pass!
            vk::SubpassEndInfo subpassEndInfo;
            currentCommandBuffer.endRenderPass2(subpassEndInfo);
            
            // Begin second render pass
            vk::RenderPassBeginInfo renderPassBeginInfo{};
            vk::SubpassBeginInfo subpassBeginInfo;
            subpassBeginInfo.setContents(vk::SubpassContents::eInline);
            renderPassBeginInfo.setRenderPass(this->renderPassImgui);
            renderPassBeginInfo.renderArea.setExtent(this->swapchain.getVkExtent());
            renderPassBeginInfo.renderArea.setOffset(vk::Offset2D(0, 0));
            renderPassBeginInfo.setFramebuffer(this->frameBuffersImgui[imageIndex]);
            renderPassBeginInfo.setClearValueCount(uint32_t(0));

            currentCommandBuffer.beginRenderPass2(&renderPassBeginInfo, &subpassBeginInfo);

            // Viewport
            viewport = vk::Viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = (float)this->swapchain.getWidth();
            viewport.height = (float)swapchain.getHeight();
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            currentCommandBuffer.setViewport(0, 1, &viewport);

            // Reuse the previous scissor
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
        TracyVkCollect(this->tracyContext[imageIndex],
            this->commandBuffers[imageIndex]);
        #endif
    }

    // Stop recording to a command buffer
    currentCommandBuffer.end();
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

	this->descriptorPoolImgui = this->getVkDevice().createDescriptorPool(pool_info, nullptr);
    
    IMGUI_CHECKVERSION(); 
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    
    io.ConfigWindowsResizeFromEdges = true;
    
    ImGui::StyleColorsDark();
    this->window->initImgui();

    ImGui_ImplVulkan_InitInfo imguiInitInfo {};
    imguiInitInfo.Instance = this->instance.getVkInstance();
    imguiInitInfo.PhysicalDevice = this->physicalDevice.getVkPhysicalDevice();
    imguiInitInfo.Device = this->getVkDevice();
    imguiInitInfo.QueueFamily = this->queueFamilies.getGraphicsIndex();
    imguiInitInfo.Queue = this->queueFamilies.getGraphicsQueue();
    imguiInitInfo.PipelineCache = VK_NULL_HANDLE;       //TODO: Imgui Pipeline Should have its own Cache! 
    imguiInitInfo.DescriptorPool = this->descriptorPoolImgui;
    imguiInitInfo.Subpass = 0; 
    imguiInitInfo.MinImageCount = this->swapchain.getNumMinimumImages();
    imguiInitInfo.ImageCount = this->swapchain.getNumImages();
    imguiInitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;  //TODO: check correctness    
    imguiInitInfo.Allocator = nullptr;                  //TODO: Can/should I pass in something VMA related here?
    imguiInitInfo.CheckVkResultFn = checkVkResult;
    ImGui_ImplVulkan_Init(&imguiInitInfo, this->renderPassImgui);

    this->createFramebufferImgui();

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

void VulkanRenderer::createFramebufferImgui()
{
    std::vector<vk::ImageView> attachment;    
    attachment.resize(1);
    this->frameBuffersImgui.resize(this->swapchain.getNumImages());
    for(size_t i = 0; i < this->frameBuffersImgui.size(); i++)
    {        
        attachment[0] = this->swapchain.getImageView(i); 
        this->createFramebuffer(
            this->frameBuffersImgui[i], 
            attachment, 
            this->renderPassImgui, 
            this->swapchain.getVkExtent(), 
            std::string("frameBuffers_imgui["+std::to_string(i)+"]")
        );
    }
}

void VulkanRenderer::cleanupFramebufferImgui()
{
    for (auto framebuffer: this->frameBuffersImgui) 
    {
        this->getVkDevice().destroyFramebuffer(framebuffer);        
    }
}
