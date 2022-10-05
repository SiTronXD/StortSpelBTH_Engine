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
#include "../application/Time.hpp"
#include "../components/MeshComponent.hpp"
#include "../dev/Log.hpp"
#include "../ResourceManagement/ResourceManager.hpp"
#include "../ResourceManagement/loaders/MeshLoader.hpp"
#include "../ResourceManagement/loaders/TextureLoader.hpp"

#include "glm/gtx/quaternion.hpp"

static void checkVkResult(VkResult err)
{
    if (err == 0)
        return;

    Log::error("Vulkan error from imgui: " + std::to_string(err));
}

void VulkanRenderer::initResourceManager()
{
    this->resourceManager->init(
        &this->vma,
        &this->physicalDevice.getVkPhysicalDevice(),
        &this->device,
        &this->queueFamilies.getGraphicsQueue(),
        &this->commandPool,
        this); /// TODO:  <-- REMOVE THIS, temporary used before making createTexture part of resourceManager...
}

using namespace vengine_helper::config;
int VulkanRenderer::init(Window* window, std::string&& windowName, ResourceManager* resourceMan)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif        

    this->resourceManager = resourceMan;
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
        this->swapchain.createFramebuffers(this->renderPassBase);
        this->createCommandPool();
        this->commandBuffers.createCommandBuffers(
            this->device, 
            this->commandPool, 
            MAX_FRAMES_IN_FLIGHT
        );

        this->createTextureSampler();

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
        this->resourceManager->addTexture("missing_texture.png");

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

    this->resourceManager->cleanup(); //TODO: Rewrite cleanup, "sartCleanup"
    
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

    this->getVkDevice().destroySampler(this->textureSampler);

    for(size_t i = 0; i < this->textureImages.size();i++)
    {
        this->getVkDevice().destroyImageView(this->textureImageViews[i]);
        this->getVkDevice().destroyImage(this->textureImages[i]);
        vmaFreeMemory(this->vma,this->textureImageMemory[i]);
    }

    for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        this->getVkDevice().destroySemaphore(this->renderFinished[i]);
        this->getVkDevice().destroySemaphore(this->imageAvailable[i]);
        this->getVkDevice().destroyFence(this->drawFences[i]);        
    }

    this->getVkDevice().destroyCommandPool(this->commandPool);
    
    this->animPipeline.cleanup();
    this->animShaderInput.cleanup();

    this->pipeline.cleanup();
    this->shaderInput.cleanup();

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
    this->shaderInput.updateUniformBuffer(
        this->viewProjectionUB,
        (void*)&this->uboViewProjection,
        this->currentFrame
    );
    this->animShaderInput.updateUniformBuffer(
        this->viewProjectionUB,
        (void*)&this->uboViewProjection,
        this->currentFrame
    );

    glm::mat4 testMats[2];
    testMats[0] = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, 1.0f, 1.0f));
    testMats[1] = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    this->shaderInput.updateStorageBuffer(
        this->testSB, 
        (void*)&testMats[0], 
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
        
        vk::SemaphoreSubmitInfo waitSemaphoreSubmitInfo;
        waitSemaphoreSubmitInfo.setSemaphore(this->imageAvailable[this->currentFrame]);
        waitSemaphoreSubmitInfo.setStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput);         
        // waitSemaphoreSubmitInfo.setDeviceIndex(uint32_t(1));                    // 0: sets all devices in group 1 to valid... bad or good?

        vk::SemaphoreSubmitInfo signalSemaphoreSubmitInfo;
        signalSemaphoreSubmitInfo.setSemaphore(this->renderFinished[this->currentFrame]);
        signalSemaphoreSubmitInfo.setStageMask(vk::PipelineStageFlags2());      // Stages to check semaphores at    

        std::vector<vk::CommandBufferSubmitInfo> commandBufferSubmitInfos
        {
            vk::CommandBufferSubmitInfo{
                this->commandBuffers.getCommandBuffer(this->currentFrame).
                    getVkCommandBuffer()
            }
        };        
        
        vk::SubmitInfo2 submitInfo {};      
        submitInfo.setWaitSemaphoreInfoCount(uint32_t(1));
        submitInfo.setPWaitSemaphoreInfos(&waitSemaphoreSubmitInfo);       // Pointer to the semaphore to wait on.
        submitInfo.setCommandBufferInfoCount(commandBufferSubmitInfos.size()); 
        submitInfo.setPCommandBufferInfos(commandBufferSubmitInfos.data()); // Pointer to the CommandBuffer to execute
        submitInfo.setSignalSemaphoreInfoCount(uint32_t(1));
        submitInfo.setPSignalSemaphoreInfos(&signalSemaphoreSubmitInfo);   // Semaphore that will be signaled when CommandBuffer is finished

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
	uint32_t numAnimTransforms = 0;

    auto tView = scene->getSceneReg().view<Transform, MeshComponent>();
    tView.each([&](Transform& transform, MeshComponent& meshComponent)
    {
        if (transform.scale.x > 0.9f)
        {
            meshComponent.meshID = this->resourceManager->addMesh("ghost.obj");

        }
        else
        {
            meshComponent.meshID = this->resourceManager->addMesh("Amogus/source/1.fbx");
            meshComponent.hasAnimations = true;

            numAnimTransforms =
			    this->resourceManager->getMesh(meshComponent.meshID)
			        .getMeshData()
			        .bones.size();
        }
    });

    // Engine "specifics"

	// Default shader inputs
	this->shaderInput.beginForInput(
	    this->physicalDevice,
	    this->device,
	    this->vma,
	    *this->resourceManager,
	    MAX_FRAMES_IN_FLIGHT
	);
	this->shaderInput.addPushConstant(
	    sizeof(ModelMatrix), vk::ShaderStageFlagBits::eVertex
	);
	this->viewProjectionUB =
	    this->shaderInput.addUniformBuffer(sizeof(UboViewProjection));
	this->testSB = this->shaderInput.addStorageBuffer(2 * sizeof(glm::mat4));
	this->sampler = this->shaderInput.addSampler();
	this->shaderInput.endForInput();
	this->pipeline.createPipeline(
	    this->device, this->shaderInput, this->renderPassBase
	);

	// Animation shader inputs
	this->animShaderInput.beginForInput(
	    this->physicalDevice,
	    this->device,
	    this->vma,
	    *this->resourceManager,
	    MAX_FRAMES_IN_FLIGHT
	);
	this->animShaderInput.addPushConstant(
	    sizeof(ModelMatrix), vk::ShaderStageFlagBits::eVertex
	);
	this->animViewProjectionUB =
	    this->animShaderInput.addUniformBuffer(sizeof(UboViewProjection));
	this->animTransformsSB =
	    this->animShaderInput.addStorageBuffer(
            numAnimTransforms * sizeof(glm::mat4));
	this->animSampler = this->animShaderInput.addSampler();
	this->animShaderInput.endForInput();
	this->animPipeline.createPipeline(
	    this->device, this->animShaderInput, this->renderPassBase, true
	);

    // Add all textures for possible use in the shader
    size_t numTextures = this->resourceManager->getNumTextures();
    for (size_t i = 0; i < numTextures; ++i) 
    {
        this->shaderInput.addPossibleTexture(
            i,
            this->textureSampler
        );
        this->animShaderInput.addPossibleTexture(i, this->textureSampler);
    }
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

VulkanRenderer::VulkanRenderer()
    : resourceManager(nullptr), window(nullptr)
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
    this->commandPool = this->getVkDevice().createCommandPool(poolInfo);
    VulkanDbg::registerVkObjectDbgInfo("CommandPool Presentation/Graphics", vk::ObjectType::eCommandPool, reinterpret_cast<uint64_t>(vk::CommandPool::CType(this->commandPool)));

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

void VulkanRenderer::getLerp(
    const std::vector<std::pair<float, glm::vec3>>& stamps, 
    const float& timer,
    glm::vec3& outputValue
)
{
    // Init to last value
	outputValue = stamps[stamps.size() - 1].second;

    // Try to find interval
	for (size_t i = 0; i < stamps.size() - 1; ++i)
	{
		float time1 = stamps[i  + 1].first;

        // Found interval
		if (timer < time1)
		{
			float time0 = stamps[i].first;
			float alpha = (timer - time0) / (time1 - time0);

            // Interpolate
            outputValue = glm::mix(
                stamps[i].second, 
                stamps[i + 1].second, 
                alpha);

			break;
		}
	}
}

void VulkanRenderer::getSlerp(
    const std::vector<std::pair<float, glm::quat>>& stamps, 
    const float& timer,
    glm::quat& outputValue
)
{
	// Init to last value
	outputValue = stamps[stamps.size() - 1].second;

	// Try to find interval
	for (size_t i = 0; i < stamps.size() - 1; ++i)
	{
		float time1 = stamps[i + 1].first;

		// Found interval
		if (timer < time1)
		{
			float time0 = stamps[i].first;
			float alpha = (timer - time0) / (time1 - time0);

			// Interpolate
            outputValue = 
                glm::slerp(
                    stamps[i].second, 
                    stamps[i + 1].second, 
                    alpha);

			break;
		}
	}
}

glm::mat4 VulkanRenderer::getLocalBoneTransform(
    const Bone& bone, 
    const float& timer) 
{
    // Translation
	glm::vec3 translation;
    this->getLerp(bone.translationStamps, timer, translation);
	
    // Rotation
	glm::quat rotation;
	this->getSlerp(bone.rotationStamps, timer, rotation);

    /*if(bone.parentIndex >= 0)
        rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);*/

    // Scale
	glm::vec3 scale;
	this->getLerp(bone.scaleStamps, timer, scale);

    // Final transform
	glm::mat4 finalTransform = 
        glm::translate(glm::mat4(1.0f), translation) * 
        glm::toMat4(rotation) *
        glm::scale(glm::mat4(1.0f), scale);

    return finalTransform;
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
    

    CommandBuffer& currentCommandBuffer =
        this->commandBuffers.getCommandBuffer(this->currentFrame);

    // Start recording commands to commandBuffer!
    currentCommandBuffer.begin(bufferBeginInfo);
    {   // Scope for Tracy Vulkan Zone...
        #ifndef VENGINE_NO_PROFILING
        TracyVkZone(
            this->tracyContext[imageIndex],
            this->commandBuffers.getCommandBuffer(imageIndex).getVkCommandBuffer(),
            "Render Record Commands");
        #endif
        {
        #ifndef VENGINE_NO_PROFILING        
        ZoneTransient(recordRenderPassCommands_zone2,  true); //:NOLINT   
        #endif
        
        #pragma region commandBufferRecording

            // Begin Render Pass!    
            // vk::SubpassContents::eInline; all the render commands themselves will be primary render commands (i.e. will not use secondary commands buffers)
            currentCommandBuffer.beginRenderPass2(
                renderPassBeginInfo, 
                subpassBeginInfo
            );

                vk::Viewport viewport{};
                viewport.x = 0.0f;
                viewport.y = (float)swapchain.getHeight();
                viewport.width = (float)this->swapchain.getWidth();
                viewport.height = -((float)swapchain.getHeight());
                viewport.minDepth = 0.0f;
                viewport.maxDepth = 1.0f;
                currentCommandBuffer.setViewport(viewport);

                vk::Rect2D scissor{};
                scissor.offset = vk::Offset2D{ 0, 0 };
                scissor.extent = this->swapchain.getVkExtent();
                currentCommandBuffer.setScissor(scissor);

                // Bind Pipeline to be used in render pass
                currentCommandBuffer.bindGraphicsPipeline(
                    this->pipeline
                );
                
                // For every non-animating mesh we have
                auto tView = scene->getSceneReg().view<Transform, MeshComponent>();
                tView.each([&](const Transform& transform, const MeshComponent& meshComponent)
                    {
                        if (!meshComponent.hasAnimations)
                        {
                            auto& currModel =
                                this->resourceManager->getMesh(meshComponent.meshID);

                            const glm::mat4& modelMatrix = transform.matrix;

                            // "Push" Constants to given Shader Stage Directly (using no Buffer...)
                            currentCommandBuffer.pushConstant(
                                this->shaderInput, (void*)&modelMatrix
                            );

                            // Bind vertex buffer
                            currentCommandBuffer.bindVertexBuffers2(
                                currModel.getVertexBuffer()
                            );

                            // Bind index buffer
                            currentCommandBuffer.bindIndexBuffer(
                                currModel.getIndexBuffer()
                            );

                            const std::vector<SubmeshData>& submeshes =
                                currModel.getSubmeshData();
                            for (size_t i = 0; i < submeshes.size(); ++i)
                            {
                                const SubmeshData& currentSubmesh = submeshes[i];

                                // Update for descriptors
                                this->shaderInput.setCurrentFrame(this->currentFrame);
                                this->shaderInput.setTexture(
                                    this->sampler, currentSubmesh.materialIndex
                                );
                                currentCommandBuffer.bindShaderInput(this->shaderInput);

                                // Draw
                                currentCommandBuffer.drawIndexed(
                                    currentSubmesh.numIndicies, 1, currentSubmesh.startIndex
                                );
                            }
                        }
                    }
                );

                // Bind Pipeline to be used in render pass
                currentCommandBuffer.bindGraphicsPipeline(this->animPipeline);

			    tempTimer += Time::getDT() * 24.0f;
			    if (tempTimer >= 60.0f)
			    {
				    tempTimer = 0.0f;
                }

                // For every animating mesh we have
                tView.each(
                    [&](const Transform& transform,
                        const MeshComponent& meshComponent)
                    {
                        if (meshComponent.hasAnimations)
                        {
                            Mesh& currentMesh =
                                this->resourceManager->getMesh(meshComponent.meshID);

                            MeshData& currentMeshData =
					            currentMesh.getMeshData();

                            // Transformations
					        glm::mat4* boneTransforms =
					            new glm::mat4[currentMeshData.bones.size()];
					        for (size_t i = 0; i < currentMeshData.bones.size();
					             ++i)
					        {
						        Bone& currentBone = currentMeshData.bones[i];

                                // Reset to identity matrix
						        glm::mat4 finalTransform =
						            currentBone.inverseBindPoseMatrix;
						        glm::mat4 deltaTransform = glm::mat4(1.0f);

                                // Apply transformation from parent
						        if (currentBone.parentIndex >= 0)
						        {
							        deltaTransform =
							            currentMeshData
							                .bones[currentBone.parentIndex]
							                .finalMatrix;
                                }

                                // Apply this local transformation
						        deltaTransform = 
                                    deltaTransform *
                                    this->getLocalBoneTransform(
						                currentBone,
                                        tempTimer
						            );

                                currentBone.finalMatrix = deltaTransform;

                                // Apply inverse bind and local transform
						        finalTransform =
                                    deltaTransform * finalTransform;

                                // Apply final transform to array element
						        boneTransforms[i] = finalTransform;
						        // boneTransforms[i] = glm::mat4(1.0f);
					        }

					        this->animShaderInput.updateStorageBuffer(
					            this->animTransformsSB,
					            (void *)&boneTransforms[0],
					            this->currentFrame
					        );
					        delete[] boneTransforms;




                            const glm::mat4& modelMatrix = transform.matrix;

                            // "Push" Constants to given Shader Stage Directly (using no Buffer...)
                            currentCommandBuffer.pushConstant(
                                this->animShaderInput, (void*)&modelMatrix
                            );

                            // Bind vertex buffer
                            currentCommandBuffer.bindVertexBuffers2(
					            currentMesh.getVertexBuffer()
                            );

                            // Bind index buffer
                            currentCommandBuffer.bindIndexBuffer(
					            currentMesh.getIndexBuffer()
                            );

                            const std::vector<SubmeshData>& submeshes =
					            currentMesh.getSubmeshData();
                            for (size_t i = 0; i < submeshes.size(); ++i)
                            {
                                const SubmeshData& currentSubmesh = submeshes[i];

                                // Update for descriptors
                                this->animShaderInput.setCurrentFrame(
                                    this->currentFrame
                                );
                                this->animShaderInput.setTexture(
                                    this->animSampler, 
                                    currentSubmesh.materialIndex
                                );
                                currentCommandBuffer.bindShaderInput(
                                    this->animShaderInput
                                );

                                // Draw
                                currentCommandBuffer.drawIndexed(
                                    currentSubmesh.numIndicies, 1, currentSubmesh.startIndex
                                );
                            }
                        }
                    }
                );

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

            currentCommandBuffer.beginRenderPass2(
                renderPassBeginInfo, 
                subpassBeginInfo
            );

                // Viewport
                viewport = vk::Viewport{};
                viewport.x = 0.0f;
                viewport.y = 0.0f;
                viewport.width = (float)this->swapchain.getWidth();
                viewport.height = (float)swapchain.getHeight();
                viewport.minDepth = 0.0f;
                viewport.maxDepth = 1.0f;
                currentCommandBuffer.setViewport(viewport);

                // Reuse the previous scissor
                currentCommandBuffer.setScissor(scissor);

                ImGui_ImplVulkan_RenderDrawData(
                    ImGui::GetDrawData(), 
                    currentCommandBuffer.getVkCommandBuffer()
                );

            // End second render pass
            vk::SubpassEndInfo imgui_subpassEndInfo;
            currentCommandBuffer.endRenderPass2(imgui_subpassEndInfo);
        }
        #pragma endregion commandBufferRecording
        
        #ifndef VENGINE_NO_PROFILING
        TracyVkCollect(
            this->tracyContext[imageIndex],
            currentCommandBuffer.getVkCommandBuffer()
        );
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
            this->commandBuffers.getCommandBuffer(i).getVkCommandBuffer(),
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
    this->getVkDevice().resetCommandPool(this->commandPool);
    
    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    const vk::CommandBuffer& firstCommandBuffer = 
        this->commandBuffers.getCommandBuffer(0).getVkCommandBuffer();
    firstCommandBuffer.begin(begin_info);
    
    ImGui_ImplVulkan_CreateFontsTexture(firstCommandBuffer);

    firstCommandBuffer.end();
    
    vk::SubmitInfo end_info = {};        
    end_info.commandBufferCount = 1;
    end_info.pCommandBuffers = &firstCommandBuffer;
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
