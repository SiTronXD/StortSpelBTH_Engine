#include "pch.h"
#include "VulkanRenderer.hpp"
#include "Utilities.hpp"
#include "assimp/Importer.hpp"
#include "vulkan/VulkanValidation.hpp"
#include "../dev/defs.hpp"
#include "../dev/tracyHelper.hpp"
#include "../resource_management/Configurator.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <fstream>
#include <limits>
#include <algorithm>
#include "Texture.hpp"
#include "Buffer.hpp"

#include "../application/Input.hpp"
#include "../application/Scene.hpp"
#include "../application/Time.hpp"
#include "../dev/Log.hpp"
#include "../resource_management/ResourceManager.hpp"
#include "../resource_management/loaders/MeshLoader.hpp"
#include "../resource_management/loaders/TextureLoader.hpp"

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
        &this->physicalDevice,
        &this->device,
        &this->queueFamilies.getGraphicsQueue(),
        &this->commandPool
    );
}

using namespace vengine_helper::config;
int VulkanRenderer::init(
    Window* window, 
    std::string&& windowName, 
    ResourceManager* resourceMan,
    UIRenderer* uiRenderer,
    DebugRenderer* debugRenderer)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif        

    this->resourceManager = resourceMan;
    this->uiRenderer = uiRenderer;
    this->debugRenderer = debugRenderer;
    this->window = window;

    try 
    {
        // Print out a clear message if validation layers are enabled
        if (isValidationLayersEnabled())
        {
            Log::write("Validation layers are enabled.\n");
        }

        // Create the vulkan instance
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

        // Command pools/buffers
        this->createCommandPool(
            this->queueFamilies.getGraphicsIndex(),
            this->commandPool
        );
        this->createCommandPool(
            this->queueFamilies.getComputeIndex(),
            this->computeCommandPool
        );
        this->commandBuffers.createCommandBuffers(
            this->device,
            this->commandPool,
            MAX_FRAMES_IN_FLIGHT
        );
        this->swapchainCommandBuffers.createCommandBuffers(
            this->device,
            this->commandPool,
            MAX_FRAMES_IN_FLIGHT
        );

        // Render passes
        this->renderPassBase.createRenderPassBase(
            this->device, 
            PostProcessHandler::HDR_FORMAT,
            Texture::getDepthBufferFormat(this->physicalDevice)
        );
        this->renderPassSwapchain.createRenderPassSwapchain(
            this->device,
            this->swapchain
        );
        this->renderPassImgui.createRenderPassImgui(
            this->device, 
            this->swapchain
        );

        // Framebuffers
        this->swapchain.createFramebuffers(this->renderPassSwapchain);

        this->createSynchronisation();        

#ifndef VENGINE_NO_PROFILING
        this->initTracy();
#endif
        this->initImgui();

        this->initResourceManager();

        // Setup Fallback Texture: Let first Texture be default if no other texture is found.
        uint32_t missingTextureIndex = 
            this->resourceManager->addTexture(
                DEF<std::string>(P_TEXTURES) + "missing_texture.jpg"
            );
        uint32_t blackTextureIndex =
            this->resourceManager->addTexture(
                DEF<std::string>(P_TEXTURES) + "Black.jpg"
            );
        this->resourceManager->addMaterial(
            missingTextureIndex,
            missingTextureIndex,
            blackTextureIndex
        );
        this->resourceManager->addMesh(DEF<std::string>(P_MODELS) + "cube.obj");

        // Create ui renderer
        this->uiRenderer->create(
            this->physicalDevice, 
            this->device,
            this->vma,
            *this->resourceManager,
            this->renderPassSwapchain,
            MAX_FRAMES_IN_FLIGHT
        );

        // Create debug renderer
        this->debugRenderer->create(
            this->physicalDevice,
            this->device,
            this->vma,
            *this->resourceManager,
            this->renderPassSwapchain,
            this->queueFamilies.getGraphicsQueue(),
            this->commandPool,
            MAX_FRAMES_IN_FLIGHT
        );
    }
    catch(std::runtime_error &e)
    {
        std::cout << "ERROR: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    // Light handler
    this->lightHandler.init(
        this->physicalDevice,
        this->device,
        this->vma,
        this->commandPool,
        *this->resourceManager,
        MAX_FRAMES_IN_FLIGHT
    );

    // Post process handler
    this->postProcessHandler.init(
        this->physicalDevice,
        this->device,
        this->vma,
        this->renderPassBase,
        this->queueFamilies.getGraphicsQueue(),
        this->commandPool,
        *this->resourceManager,
        MAX_FRAMES_IN_FLIGHT,
        this->swapchain.getVkExtent()
    );

    // Particle system handler
    this->particleHandler.init(
        this->physicalDevice,
        this->device,
        this->vma,
        this->queueFamilies.getGraphicsQueue(),
        this->commandPool,
        *this->resourceManager,
        this->renderPassBase,
        this->computeCommandPool,
        MAX_FRAMES_IN_FLIGHT
    );

    // Render-to-Swapchain shader input and pipeline
    this->swapchainShaderInput.beginForInput(
        this->physicalDevice,
        this->device,
        this->vma,
        *this->resourceManager,
        MAX_FRAMES_IN_FLIGHT
    );

    this->bloomSettingsUB = this->swapchainShaderInput.addUniformBuffer(
        sizeof(BloomSettingsBufferData),
        vk::ShaderStageFlagBits::eFragment,
        DescriptorFrequency::PER_FRAME
    );

    // Layout
    FrequencyInputLayout freqInputLayout{};
    freqInputLayout.addBinding(vk::DescriptorType::eCombinedImageSampler);
    freqInputLayout.addBinding(vk::DescriptorType::eCombinedImageSampler);
    this->swapchainShaderInput.makeFrequencyInputLayout(freqInputLayout);

    this->swapchainShaderInput.endForInput();
    this->swapchainPipeline.createPipeline(
        this->device, 
        this->swapchainShaderInput, 
        this->renderPassSwapchain,
        VertexStreams{},
        "fullscreenQuad.vert.spv",
        "hdrToSwapchain.frag.spv",
        false
    );

    // Input
    FrequencyInputBindings freqInputBinding0{};
    freqInputBinding0.texture = &this->postProcessHandler.getHdrRenderTexture();
    FrequencyInputBindings freqInputBinding1{};
    freqInputBinding1.texture = &this->postProcessHandler.getHdrRenderTexture();
    freqInputBinding1.imageView = &this->postProcessHandler.getHdrRenderTexture().getMipImageView(1);
    this->hdrRenderTextureDescriptorIndex =
        this->swapchainShaderInput.addFrequencyInput(
            { 
                freqInputBinding0,
                freqInputBinding1
            }
        );
    
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
    
    // Wait until no actions is run on device...
    this->device.waitIdle(); // Dont destroy semaphores before they are done

    this->resourceManager->cleanup(); //TODO: Rewrite cleanup, "sartCleanup"
    
    ImGui_ImplVulkan_Shutdown();
    this->window->shutdownImgui();
    ImGui::DestroyContext();

    this->frameBuffersImgui.cleanup();

    this->renderPassImgui.cleanup();
    this->getVkDevice().destroyDescriptorPool(this->descriptorPoolImgui);
    
#ifndef VENGINE_NO_PROFILING
    CustomFree(this->tracyImage);

    for(auto &tracy_context : this->tracyContext)
    {
        TracyVkDestroy(tracy_context);
    }
#endif

    for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        this->getVkDevice().destroySemaphore(this->swapchainRenderFinished[i]);

        for (size_t j = 0; j < this->downsampleFinished[i].size(); ++j)
        {
            this->getVkDevice().destroySemaphore(this->downsampleFinished[i][j]);
        }
        this->downsampleFinished[i].clear();

        for (size_t j = 0; j < this->upsampleFinished[i].size(); ++j)
        {
            this->getVkDevice().destroySemaphore(this->upsampleFinished[i][j]);
        }
        this->upsampleFinished[i].clear();

        this->getVkDevice().destroySemaphore(this->sceneRenderFinished[i]);
        this->getVkDevice().destroySemaphore(this->shadowMapRenderFinished[i]);
        this->getVkDevice().destroySemaphore(this->computeFinished[i]);
        this->getVkDevice().destroySemaphore(this->imageAvailable[i]);
        this->getVkDevice().destroyFence(this->drawFences[i]);
    }

    this->getVkDevice().destroyCommandPool(this->computeCommandPool);
    this->getVkDevice().destroyCommandPool(this->commandPool);
    
    this->debugRenderer->cleanup();
    this->uiRenderer->cleanup();

    this->swapchainPipeline.cleanup();
    this->swapchainShaderInput.cleanup();

    if (this->hasAnimations)
	{
		this->animPipeline.cleanup();
		this->animShaderInput.cleanup();
    }

    this->pipeline.cleanup();
    this->shaderInput.cleanup();

    this->particleHandler.cleanup();
    this->postProcessHandler.cleanup();
    this->lightHandler.cleanup(this->hasAnimations);

    this->renderPassSwapchain.cleanup();
    this->renderPassBase.cleanup();
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
    // Apply bloom settings from the scene
    const BloomSettings& sceneBloomSettings = 
        scene->getBloomSettings();

    this->bloomSettingsData.strength.x = 
        std::clamp(sceneBloomSettings.bloomBufferLerpAlpha, 0.0f, 1.0f);
    this->postProcessHandler.setDesiredNumMipLevels(
        sceneBloomSettings.numBloomMipLevels
    );

    // Apply fog settings from the scene
    const FogSettings& fogSettings =
        scene->getFogSettings();

    this->pushConstantData.settings.y = fogSettings.fogStartDist;
    this->pushConstantData.settings.z = std::max(fogSettings.fogAbsorption, 0.0f);

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
        
        // TODO: PROFILING; Check if its faster to have wait for fences after acquire image or not...
        // Wait for The Fence to be signaled from last Draw for this currrent Frame; 
        // This will freeze the CPU operations here and wait for the Fence to open
        vk::Bool32 waitForAllFences = VK_TRUE;

        auto result = this->getVkDevice().waitForFences(
            uint32_t(1),                          // Number of Fences to wait on
            &this->drawFences[this->currentFrame],// Which Fences to wait on
            waitForAllFences,                     // Should we wait for all Fences or not?
            std::numeric_limits<uint64_t>::max());
        if(result != vk::Result::eSuccess) 
        {
            Log::error("Failed to wait for all fences!");
        }
    }

    // Get scene camera and update view matrix
    Camera* camera = scene->getMainCamera();
    Transform* cameraTransform = nullptr;
    bool deleteCamera = false;
    if (camera)
    {
        cameraTransform = &scene->getComponent<Transform>(scene->getMainCameraID());
        camera->updateMatrices(*cameraTransform);

        if (!scene->isActive(scene->getMainCameraID()))
        {
            Log::warning("Main camera is inactive!");
        }
    }
    else
    {
        Log::error("No main camera exists!");
        camera = new Camera((float)this->swapchain.getWidth() / (float)this->swapchain.getHeight());
        camera->view = cameraDataUBO.view;

        cameraTransform = new Transform();

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
        if(result == vk::Result::eErrorOutOfDateKHR)
        {
            this->windowResize(camera);
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
    this->cameraDataUBO.projection = camera->projection;
    this->cameraDataUBO.view = camera->view;
    this->cameraDataUBO.worldPosition = glm::vec4(cameraTransform->position, 1.0f);
    
    // Record command buffers
    this->recordCommandBuffers(scene, camera, imageIndex);

    if (deleteCamera)
    {
        delete camera;
        delete cameraTransform;
    }
    
    // Submit to queues
    {
        // Reset submit arrays
        this->computeSubmitArray.reset();
        this->graphicsSubmitArray.reset();

        // ---------- Compute submit ----------
        std::array<vk::SemaphoreSubmitInfo, 4> computeWaitSemaphores;
        this->computeSubmitArray.setSubmitInfo(
            *this->currentComputeCommandBuffer,
            computeWaitSemaphores,
            0,
            this->computeFinished[this->currentFrame]
        );

        vk::Result computeQueueResult =
            this->queueFamilies.getComputeQueue().submit2(
                this->computeSubmitArray.getNumSubmits(),
                this->computeSubmitArray.getSubmitInfos().data(),
                VK_NULL_HANDLE
            );
        if (computeQueueResult != vk::Result::eSuccess)
        {
            Log::error("Failed to submit compute commands.");
        }

        // ---------- Graphics submit ----------

        // Shadow map
        std::array<vk::SemaphoreSubmitInfo, 4> shadowMapWaitSemaphores;
        this->graphicsSubmitArray.setSubmitInfo(
            *this->currentShadowMapCommandBuffer,
            shadowMapWaitSemaphores,
            0,
            this->shadowMapRenderFinished[this->currentFrame]
        );

        // Render to screen
        std::array<vk::SemaphoreSubmitInfo, 4> renderToScreenWaitSemaphores;
        renderToScreenWaitSemaphores[0].setSemaphore(this->shadowMapRenderFinished[this->currentFrame]);
        renderToScreenWaitSemaphores[0].setStageMask(vk::PipelineStageFlagBits2::eFragmentShader);
        renderToScreenWaitSemaphores[1].setSemaphore(this->computeFinished[this->currentFrame]);
        renderToScreenWaitSemaphores[1].setStageMask(vk::PipelineStageFlagBits2::eVertexShader);
        this->graphicsSubmitArray.setSubmitInfo(
            *this->currentCommandBuffer,
            renderToScreenWaitSemaphores,
            2,
            this->sceneRenderFinished[this->currentFrame]
        );

        // Bloom downsampling
        std::array<std::array<vk::SemaphoreSubmitInfo, 4>, PostProcessHandler::MAX_NUM_MIP_LEVELS> bloomDownsampleWaitSemaphores;
        bloomDownsampleWaitSemaphores[1][0].setSemaphore(this->sceneRenderFinished[this->currentFrame]);
        bloomDownsampleWaitSemaphores[1][0].setStageMask(vk::PipelineStageFlagBits2::eFragmentShader);
        for (uint32_t i = 1; i < this->postProcessHandler.getNumMipLevelsInUse(); ++i)
        {
            if (i > 1)
            {
                bloomDownsampleWaitSemaphores[i][0].setSemaphore(this->downsampleFinished[this->currentFrame][i - 1]);
                bloomDownsampleWaitSemaphores[i][0].setStageMask(vk::PipelineStageFlagBits2::eFragmentShader);
            }

            this->graphicsSubmitArray.setSubmitInfo(
                this->postProcessHandler.getDownsampleCommandBuffer(this->currentFrame, i),
                bloomDownsampleWaitSemaphores[i],
                1,
                this->downsampleFinished[this->currentFrame][i]
            );
        }

        // Bloom upsampling
        std::array<std::array<vk::SemaphoreSubmitInfo, 4>, PostProcessHandler::MAX_NUM_MIP_LEVELS> bloomUpsampleWaitSemaphores;
        bloomUpsampleWaitSemaphores[this->postProcessHandler.getNumMipLevelsInUse() - 2][0].setSemaphore(this->downsampleFinished[this->currentFrame][this->postProcessHandler.getNumMipLevelsInUse() - 1]);
        bloomUpsampleWaitSemaphores[this->postProcessHandler.getNumMipLevelsInUse() - 2][0].setStageMask(vk::PipelineStageFlagBits2::eFragmentShader);
        for (uint32_t i = this->postProcessHandler.getNumMipLevelsInUse() - 2; i >= 1; --i)
        {
            if (i < this->postProcessHandler.getNumMipLevelsInUse() - 2)
            {
                bloomUpsampleWaitSemaphores[i][0].setSemaphore(this->upsampleFinished[this->currentFrame][i + 1]);
                bloomUpsampleWaitSemaphores[i][0].setStageMask(vk::PipelineStageFlagBits2::eFragmentShader);
            }

            this->graphicsSubmitArray.setSubmitInfo(
                this->postProcessHandler.getUpsampleCommandBuffer(this->currentFrame, i),
                bloomUpsampleWaitSemaphores[i],
                1,
                this->upsampleFinished[this->currentFrame][i]
            );
        }

        // Render to swapchain
        std::array<vk::SemaphoreSubmitInfo, 4> renderToSwapchainWaitSemaphores;
        renderToSwapchainWaitSemaphores[0].setSemaphore(this->upsampleFinished[this->currentFrame][1]);
        renderToSwapchainWaitSemaphores[0].setStageMask(vk::PipelineStageFlagBits2::eFragmentShader);
        renderToSwapchainWaitSemaphores[1].setSemaphore(this->imageAvailable[this->currentFrame]);
        renderToSwapchainWaitSemaphores[1].setStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput);
        this->graphicsSubmitArray.setSubmitInfo(
            *this->currentSwapchainCommandBuffer,
            renderToSwapchainWaitSemaphores,
            2,
            this->swapchainRenderFinished[this->currentFrame]
        );

        // Submit to graphics queue
        vk::Result graphicsQueueResult = 
            this->queueFamilies.getGraphicsQueue().submit2(
                this->graphicsSubmitArray.getNumSubmits(),
                this->graphicsSubmitArray.getSubmitInfos().data(),
                this->drawFences[this->currentFrame]
            );
        if (graphicsQueueResult != vk::Result::eSuccess)
        {
            Log::error("Failed to submit graphics commands.");
        }
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
        presentInfo.setPWaitSemaphores(&this->swapchainRenderFinished[this->currentFrame]);  // Semaphore to Wait on before Presenting
        presentInfo.setSwapchainCount(uint32_t (1));    
        presentInfo.setPSwapchains(&this->swapchain.getVkSwapchain());              // Swapchain to present the image to
        presentInfo.setPImageIndices(&imageIndex);                                  // Index of images in swapchains to present                

        // Submit the image to the presentation Queue
        vk::Result resultvk = 
            this->queueFamilies.getPresentQueue().presentKHR(&presentInfo);
        if (resultvk == vk::Result::eErrorOutOfDateKHR || resultvk == vk::Result::eSuboptimalKHR || this->windowResized )
        {
            this->windowResized = false;       
            this->windowResize(camera);
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

void VulkanRenderer::initForScene(Scene* scene)
{
    // Wait idle before doing anything
    this->device.waitIdle();

    bool oldHasAnimations = this->hasAnimations;

    // Try to cleanup before creating new objects
    this->shaderInput.cleanup();
    this->pipeline.cleanup();
    if (oldHasAnimations) // (hasAnimations from previous scene)
    {
        this->animShaderInput.cleanup();
        this->animPipeline.cleanup();
    }

    // UI renderer
    this->uiRenderer->initForScene();

    // Debug renderer
    this->debugRenderer->initForScene();

	// Default shader inputs
    VertexStreams defaultStream{};
    defaultStream.positions.resize(1);
    defaultStream.normals.resize(1);
    defaultStream.texCoords.resize(1);

    // Default per draw bindings
    FrequencyInputLayout perDrawInputLayout{};
    perDrawInputLayout.addBinding(vk::DescriptorType::eCombinedImageSampler);
    perDrawInputLayout.addBinding(vk::DescriptorType::eCombinedImageSampler);
    perDrawInputLayout.addBinding(vk::DescriptorType::eCombinedImageSampler);
    
	this->shaderInput.beginForInput(
	    this->physicalDevice,
	    this->device,
	    this->vma,
	    *this->resourceManager,
	    MAX_FRAMES_IN_FLIGHT
	);
	this->shaderInput.addPushConstant(
	    sizeof(PushConstantData), 
        vk::ShaderStageFlagBits::eVertex
	);
	this->viewProjectionUB =
	    this->shaderInput.addUniformBuffer(
            sizeof(CameraBufferData),
            vk::ShaderStageFlagBits::eVertex,
            DescriptorFrequency::PER_FRAME);
    this->allLightsInfoUB =
        this->shaderInput.addUniformBuffer(
            sizeof(AllLightsInfo), 
            vk::ShaderStageFlagBits::eFragment,
            DescriptorFrequency::PER_FRAME
        );
    this->lightBufferSB = 
        this->shaderInput.addStorageBuffer(
            sizeof(LightBufferData) * LightHandler::MAX_NUM_LIGHTS,
            vk::ShaderStageFlagBits::eFragment,
            DescriptorFrequency::PER_FRAME
        );
    this->shaderInput.addCombinedImageSampler(
        this->lightHandler.getShadowMapTexture(),
        vk::ShaderStageFlagBits::eFragment,
        DescriptorFrequency::PER_FRAME
    );
    this->shadowMapDataUB =
        this->shaderInput.addUniformBuffer(
            sizeof(ShadowMapData),
            vk::ShaderStageFlagBits::eFragment,
            DescriptorFrequency::PER_FRAME
        );
    this->shaderInput.makeFrequencyInputLayout(
        // DescriptorFrequency::PER_DRAW_CALL, 
        perDrawInputLayout
    );
	this->shaderInput.endForInput();
	this->pipeline.createPipeline(
	    this->device, 
        this->shaderInput, 
        this->renderPassBase,
        defaultStream,
        "shader.vert.spv"
	);

	// Animation shader inputs
	uint32_t numAnimMeshes = 0;
	auto tView =
	    scene->getSceneReg().view<Transform, MeshComponent, AnimationComponent>();
	tView.each(
	    [&](const Transform& transform,
	        const MeshComponent& meshComponent,
	        AnimationComponent& animationComponent) 
        { 
            numAnimMeshes++;
	    }
	);
	this->hasAnimations = numAnimMeshes > 0;

    // Make sure animated meshes actually exists
    if (this->hasAnimations)
	{
		VertexStreams animStream{};
		animStream.positions.resize(1);
		animStream.normals.resize(1);
		animStream.texCoords.resize(1);
		animStream.boneWeights.resize(1);
		animStream.boneIndices.resize(1);

		this->animShaderInput.beginForInput(
		    this->physicalDevice,
		    this->device,
		    this->vma,
		    *this->resourceManager,
		    MAX_FRAMES_IN_FLIGHT
		);
		this->animShaderInput.addPushConstant(
		    sizeof(PushConstantData), 
            vk::ShaderStageFlagBits::eVertex
		);
		this->animShaderInput.setNumShaderStorageBuffers(1);

		// Add shader inputs for animations
		tView.each(
		    [&](const Transform& transform,
		        const MeshComponent& meshComponent,
		        AnimationComponent& animationComponent)
		    {
			    // Extract mesh information
			    Mesh& currentMesh =
			        this->resourceManager->getMesh(meshComponent.meshID);
			    const std::vector<Bone>& bones = currentMesh.getMeshData().bones;
			    uint32_t numAnimationBones = bones.size();

                // Make sure the mesh actually has bones
                if (numAnimationBones == 0)
                {
                    Log::error("Mesh ID " + std::to_string(meshComponent.meshID) + " does not have any bones for skeletal animations. Please remove the animation component from this entity.");
                }

			    // Add new storage buffer for animations
			    StorageBufferID newStorageBufferID =
			        this->animShaderInput.addStorageBuffer(
			            numAnimationBones * sizeof(glm::mat4),
                        vk::ShaderStageFlagBits::eVertex,
                        DescriptorFrequency::PER_MESH
			        );

			    // Update animation component with storage buffer ID
			    animationComponent.boneTransformsID = newStorageBufferID;
		    }
		);
		this->animViewProjectionUB =
		    this->animShaderInput.addUniformBuffer(
                sizeof(CameraBufferData),
                vk::ShaderStageFlagBits::eVertex,
                DescriptorFrequency::PER_FRAME
            );
        this->animAllLightsInfoUB =
            this->animShaderInput.addUniformBuffer(
                sizeof(AllLightsInfo),
                vk::ShaderStageFlagBits::eFragment,
                DescriptorFrequency::PER_FRAME
            );
        this->animLightBufferSB =
            this->animShaderInput.addStorageBuffer(
                sizeof(LightBufferData) * LightHandler::MAX_NUM_LIGHTS,
                vk::ShaderStageFlagBits::eFragment,
                DescriptorFrequency::PER_FRAME
            );
        this->animShaderInput.addCombinedImageSampler(
            this->lightHandler.getShadowMapTexture(),
            vk::ShaderStageFlagBits::eFragment,
            DescriptorFrequency::PER_FRAME
        );
        this->animShadowMapDataUB =
            this->animShaderInput.addUniformBuffer(
                sizeof(ShadowMapData),
                vk::ShaderStageFlagBits::eFragment,
                DescriptorFrequency::PER_FRAME
            );
        this->animShaderInput.makeFrequencyInputLayout(
            // DescriptorFrequency::PER_DRAW_CALL,
            perDrawInputLayout
        );
		this->animShaderInput.endForInput();
		this->animPipeline.createPipeline(
		    this->device,
		    this->animShaderInput,
		    this->renderPassBase,
		    animStream,
		    "shaderAnim.vert.spv"
		);
	}

    // Add all materials for possible use in the shaders
    size_t numMaterials = this->resourceManager->getNumMaterials();
    for (size_t i = 0; i < numMaterials; ++i)
    {
        Material& material = this->resourceManager->getMaterial(i);

        FrequencyInputBindings diffuseTextureInputBinding{};
        FrequencyInputBindings specularTextureInputBinding{};
        FrequencyInputBindings glowMapTextureInputBinding{};
        diffuseTextureInputBinding.texture = &this->resourceManager->getTexture(material.diffuseTextureIndex);
        specularTextureInputBinding.texture = &this->resourceManager->getTexture(material.specularTextureIndex);
        glowMapTextureInputBinding.texture = &this->resourceManager->getTexture(material.glowMapTextureIndex);

        // Update material's descriptor index
        material.descriptorIndex =
            this->shaderInput.addFrequencyInput(
                {
                    diffuseTextureInputBinding,
                    specularTextureInputBinding,
                    glowMapTextureInputBinding
                }
        );

        if (this->hasAnimations)
        {
            // Add one descriptor in animShaderInput for 
            // each added descriptor in shaderInput
            this->animShaderInput.addFrequencyInput(
                {
                    diffuseTextureInputBinding,
                    specularTextureInputBinding,
                    glowMapTextureInputBinding
                }
            );
        }
    }

    // Add all unique materials as well for possible use in the shaders
    auto meshView =
        scene->getSceneReg().view<MeshComponent>();
    meshView.each(
        [&](MeshComponent& meshComponent)
        {
            for (uint32_t i = 0; i < meshComponent.numOverrideMaterials; ++i)
            {
                // Get material
                Material& material = meshComponent.overrideMaterials[i];

                // Make binding recognize material parameters
                FrequencyInputBindings diffuseTextureInputBinding{};
                FrequencyInputBindings specularTextureInputBinding{};
                FrequencyInputBindings glowMapTextureInputBinding{};
                diffuseTextureInputBinding.texture = &this->resourceManager->getTexture(material.diffuseTextureIndex);
                specularTextureInputBinding.texture = &this->resourceManager->getTexture(material.specularTextureIndex);
                glowMapTextureInputBinding.texture = &this->resourceManager->getTexture(material.glowMapTextureIndex);

                // Update material's descriptor index
                material.descriptorIndex =
                    this->shaderInput.addFrequencyInput(
                        {
                            diffuseTextureInputBinding,
                            specularTextureInputBinding,
                            glowMapTextureInputBinding
                        }
                );

                if (this->hasAnimations)
                {
                    // Add one descriptor in animShaderInput for 
                    // each added descriptor in shaderInput
                    this->animShaderInput.addFrequencyInput(
                        {
                            diffuseTextureInputBinding,
                            specularTextureInputBinding,
                            glowMapTextureInputBinding
                        }
                    );
                }
            }
        }
    );

    // Add all textures for possible use in the ui renderer
    size_t numTextures = this->resourceManager->getNumTextures();
    for (size_t i = 0; i < numTextures; ++i)
    {
        Texture& texture = this->resourceManager->getTexture(i);

        texture.setDescriptorIndex(
            this->uiRenderer->getShaderInput().addFrequencyInput(
                { FrequencyInputBindings{ &texture } }
            )
        );
    }

    // Set aspect ratio for the main camera
    Camera* mainCam = scene->getMainCamera();
    if (mainCam)
    {
        // Recalculate projection matrix
        mainCam->calculateProjectionMatrix(
            (float) this->swapchain.getWidth()  / this->swapchain.getHeight()
        );
    }

    this->lightHandler.initForScene(
        scene,
        oldHasAnimations,
        this->hasAnimations
    );
    this->particleHandler.initForScene(scene);
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

void VulkanRenderer::windowResize(Camera* camera)
{
    this->device.waitIdle();
    
    // Busy wait until swapchain has an actual resolution.
    // For example when the window is minimized.
    while (!this->swapchain.canCreateValidSwapchain())
    {
        this->window->update();
    }

    // Reset delta time, just to make sure not too much time 
    // has passed to create precision errors.
    Time::reset();

    // Cleanup framebuffers
    this->frameBuffersImgui.cleanup();

    // Recreate swapchain and framebuffers
    this->swapchain.recreateSwapchain(this->renderPassSwapchain);
    std::vector<std::vector<vk::ImageView>> imguiFramebufferAttachments(this->swapchain.getNumImages());
    for (size_t i = 0; i < imguiFramebufferAttachments.size(); ++i)
    {
        imguiFramebufferAttachments[i].push_back(
            this->swapchain.getImageView(i)
        );
    }
    this->frameBuffersImgui.create(
        device,
        this->renderPassImgui,
        this->swapchain.getVkExtent(),
        imguiFramebufferAttachments
    );

    ImGui_ImplVulkan_SetMinImageCount(this->swapchain.getNumMinimumImages());

    // Recalculate HDR render texture and depth buffer
    this->postProcessHandler.recreate(this->swapchain.getVkExtent());

    // Update descriptor set for HDR to swapchain image rendering
    FrequencyInputBindings freqInputBinding0{};
    freqInputBinding0.texture = &this->postProcessHandler.getHdrRenderTexture();
    FrequencyInputBindings freqInputBinding1{};
    freqInputBinding1.texture = &this->postProcessHandler.getHdrRenderTexture();
    freqInputBinding1.imageView = &this->postProcessHandler.getHdrRenderTexture().getMipImageView(1);
    this->swapchainShaderInput.updateFrequencyInput(
        {
            freqInputBinding0,
            freqInputBinding1
        },
        this->hdrRenderTextureDescriptorIndex
    );

    // Take new aspect ratio into account for the camera
    camera->calculateProjectionMatrix(
        (float) this->swapchain.getWidth() / swapchain.getHeight()
    );
}

VulkanRenderer::VulkanRenderer()
    : resourceManager(nullptr), 
    uiRenderer(nullptr), 
    debugRenderer(nullptr),
    window(nullptr),
    hasAnimations(false)
{
    loadConfIntoMemory();
}

void VulkanRenderer::createCommandPool(
    const uint32_t& queueFamilyIndex,
    vk::CommandPool& outputCommandPool)
 {
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    vk::CommandPoolCreateInfo poolInfo{};
    poolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
    poolInfo.setQueueFamilyIndex(queueFamilyIndex);

    // Create a command pool
    outputCommandPool = this->getVkDevice().createCommandPool(poolInfo);
    VulkanDbg::registerVkObjectDbgInfo("CommandPool for queue family index " + std::to_string(queueFamilyIndex), vk::ObjectType::eCommandPool, reinterpret_cast<uint64_t>(vk::CommandPool::CType(outputCommandPool)));

}

void VulkanRenderer::createSynchronisation()
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    // One semaphore/fence per frame in flight
    this->imageAvailable.resize(MAX_FRAMES_IN_FLIGHT);
    this->computeFinished.resize(MAX_FRAMES_IN_FLIGHT);
    this->shadowMapRenderFinished.resize(MAX_FRAMES_IN_FLIGHT);
    this->sceneRenderFinished.resize(MAX_FRAMES_IN_FLIGHT);
    this->swapchainRenderFinished.resize(MAX_FRAMES_IN_FLIGHT);

    this->downsampleFinished.resize(MAX_FRAMES_IN_FLIGHT);
    for (size_t i = 0; i < this->downsampleFinished.size(); ++i)
    {
        this->downsampleFinished[i].resize(PostProcessHandler::MAX_NUM_MIP_LEVELS);
    }

    this->upsampleFinished.resize(MAX_FRAMES_IN_FLIGHT);
    for (size_t i = 0; i < this->upsampleFinished.size(); ++i)
    {
        this->upsampleFinished[i].resize(PostProcessHandler::MAX_NUM_MIP_LEVELS);
    }

    this->drawFences.resize(MAX_FRAMES_IN_FLIGHT);

    // Semaphore creation information
    vk::SemaphoreCreateInfo semaphoreCreateInfo; 
    
    // Fence creation Information
    vk::FenceCreateInfo fenceCreateInfo;
    fenceCreateInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);           // Make sure the Fence is initially open!

    for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        // Semaphores
        this->imageAvailable[i] = this->getVkDevice().createSemaphore(semaphoreCreateInfo);
        VulkanDbg::registerVkObjectDbgInfo("Semaphore imageAvailable["+std::to_string(i)+"]", vk::ObjectType::eSemaphore, reinterpret_cast<uint64_t>(vk::Semaphore::CType(this->imageAvailable[i])));

        this->computeFinished[i] = this->getVkDevice().createSemaphore(semaphoreCreateInfo);
        VulkanDbg::registerVkObjectDbgInfo("Semaphore computeFinished[" + std::to_string(i) + "]", vk::ObjectType::eSemaphore, reinterpret_cast<uint64_t>(vk::Semaphore::CType(this->computeFinished[i])));

        this->shadowMapRenderFinished[i] = this->getVkDevice().createSemaphore(semaphoreCreateInfo);
        VulkanDbg::registerVkObjectDbgInfo("Semaphore shadowMapRenderFinished[" + std::to_string(i) + "]", vk::ObjectType::eSemaphore, reinterpret_cast<uint64_t>(vk::Semaphore::CType(this->shadowMapRenderFinished[i])));

        this->sceneRenderFinished[i] = this->getVkDevice().createSemaphore(semaphoreCreateInfo);
        VulkanDbg::registerVkObjectDbgInfo("Semaphore sceneRenderFinished[" + std::to_string(i) + "]", vk::ObjectType::eSemaphore, reinterpret_cast<uint64_t>(vk::Semaphore::CType(this->sceneRenderFinished[i])));
        
        for (size_t j = 0; j < this->downsampleFinished[i].size(); ++j)
        {
            this->downsampleFinished[i][j] = this->getVkDevice().createSemaphore(semaphoreCreateInfo);
            VulkanDbg::registerVkObjectDbgInfo("Semaphore bloomDownsampleFinished[" + std::to_string(i) + "][" + std::to_string(j) + "]", vk::ObjectType::eSemaphore, reinterpret_cast<uint64_t>(vk::Semaphore::CType(this->downsampleFinished[i][j])));
        }

        for (size_t j = 0; j < this->upsampleFinished[i].size(); ++j)
        {
            this->upsampleFinished[i][j] = this->getVkDevice().createSemaphore(semaphoreCreateInfo);
            VulkanDbg::registerVkObjectDbgInfo("Semaphore bloomUpsampleFinished[" + std::to_string(i) + "][" + std::to_string(j) + "]", vk::ObjectType::eSemaphore, reinterpret_cast<uint64_t>(vk::Semaphore::CType(this->upsampleFinished[i][j])));
        }

        this->swapchainRenderFinished[i] = this->getVkDevice().createSemaphore(semaphoreCreateInfo);
        VulkanDbg::registerVkObjectDbgInfo("Semaphore swapchainRenderFinished["+std::to_string(i)+"]", vk::ObjectType::eSemaphore, reinterpret_cast<uint64_t>(vk::Semaphore::CType(this->swapchainRenderFinished[i])));
        
        // Fence
        this->drawFences[i] = this->getVkDevice().createFence(fenceCreateInfo);
        VulkanDbg::registerVkObjectDbgInfo("Fence drawFences["+std::to_string(i)+"]", vk::ObjectType::eFence, reinterpret_cast<uint64_t>(vk::Fence::CType(this->drawFences[i])));
    }

    // Particles
    this->computeSubmitArray.setMaxNumSubmits(1);

    // Shadow map + scene + 
    // num bloom downsamples +
    // num bloom upsamples +
    // HDR to swapchain
    this->graphicsSubmitArray.setMaxNumSubmits(
        2 + 
        (PostProcessHandler::MAX_NUM_MIP_LEVELS - 1) +
        (PostProcessHandler::MAX_NUM_MIP_LEVELS - 2) +
        1
    );
}

const Material& VulkanRenderer::getAppropriateMaterial(
    const MeshComponent& meshComponent,
    const std::vector<SubmeshData>& submeshes,
    const uint32_t& submeshIndex)
{
    // Unique overriden materials
    if (meshComponent.numOverrideMaterials > 0)
    {
        return meshComponent.overrideMaterials[submeshIndex];
    }

    // Default mesh materials
    return this->resourceManager->getMaterial(submeshes[submeshIndex].materialIndex);
}

Material& VulkanRenderer::getAppropriateMaterial(
    MeshComponent& meshComponent,
    const std::vector<SubmeshData>& submeshes,
    const uint32_t& submeshIndex)
{
    // Unique overriden materials
    if (meshComponent.numOverrideMaterials > 0)
    {
        return meshComponent.overrideMaterials[submeshIndex];
    }

    // Default mesh materials
    return this->resourceManager->getMaterial(submeshes[submeshIndex].materialIndex);
}

void VulkanRenderer::recordCommandBuffers(
    Scene* scene, 
    Camera* camera,
    uint32_t imageIndex)
{
#ifndef VENGINE_NO_PROFILING
    //ZoneScoped; //:NOLINT     
    ZoneTransient(recordRenderPassCommands_zone1,  true); //:NOLINT   
#endif

    this->currentComputeCommandBuffer =
        &this->particleHandler
            .getComputeCommandBuffer(this->currentFrame);
    this->currentShadowMapCommandBuffer =
        &this->lightHandler
        .getShadowMapCommandBuffer(this->currentFrame);
    this->currentCommandBuffer =
        &this->commandBuffers[this->currentFrame];
    this->currentSwapchainCommandBuffer =
        &this->swapchainCommandBuffers[this->currentFrame];

    
    // Set current frame
    this->shaderInput.setCurrentFrame(this->currentFrame);
    if (hasAnimations)
    {
        this->animShaderInput.setCurrentFrame(this->currentFrame);
    }
    this->uiRenderer->getShaderInput().setCurrentFrame(
        this->currentFrame
    );
    this->debugRenderer->getLineShaderInput().setCurrentFrame(
        this->currentFrame
    );
    this->debugRenderer->getMeshShaderInput().setCurrentFrame(
        this->currentFrame
    );
    this->swapchainShaderInput.setCurrentFrame(this->currentFrame);

    // Update light buffers
    this->lightHandler.updateLightBuffers(
        scene,
        this->shaderInput,
        this->animShaderInput,
        this->allLightsInfoUB,
        this->animAllLightsInfoUB,
        this->lightBufferSB,
        this->animLightBufferSB,
        this->hasAnimations,
        glm::vec3(this->cameraDataUBO.worldPosition),
        *camera,
        this->currentFrame
    );

    // Update particles info
    this->particleHandler.update(
        scene,
        this->cameraDataUBO,
        this->currentFrame
    );

    // Default shader input
    this->shaderInput.updateUniformBuffer(
        this->viewProjectionUB,
        (void*)&this->cameraDataUBO
    );
    this->shaderInput.updateUniformBuffer(
        this->shadowMapDataUB,
        (void*) &this->lightHandler.getShadowMapData()
    );

    // Animation shader input
    if (this->hasAnimations)
	{
		this->animShaderInput.updateUniformBuffer(
			this->viewProjectionUB, 
            (void*)&this->cameraDataUBO
		);
        this->animShaderInput.updateUniformBuffer(
            this->animShadowMapDataUB,
            (void*) &this->lightHandler.getShadowMapData()
        );
	}

    // UI shader input
    this->uiRenderer->prepareForGPU();

    // Debug renderer shader input
    this->debugRenderer->prepareGPU(this->currentFrame);
    this->debugRenderer->getLineShaderInput().updateUniformBuffer(
        this->viewProjectionUB,
        (void*)&this->cameraDataUBO
    );
    this->debugRenderer->getMeshShaderInput().updateUniformBuffer(
        this->viewProjectionUB,
        (void*)&this->cameraDataUBO
    );

    this->swapchainShaderInput.updateUniformBuffer(
        this->bloomSettingsUB,
        (void*) &this->bloomSettingsData
    );

    // Particle compute
    this->currentComputeCommandBuffer->beginOneTimeSubmit();
        this->computeParticles();
    this->currentComputeCommandBuffer->end();

    // Begin shadow map command buffer
    this->currentShadowMapCommandBuffer->beginOneTimeSubmit();
            
    // Render shadow map
    for (uint32_t i = 0; i < LightHandler::NUM_CASCADES; ++i)
    {
        this->beginShadowMapRenderPass(
            this->lightHandler,
            i
        );
        this->renderShadowMapDefaultMeshes(scene, this->lightHandler);
        this->renderShadowMapSkeletalAnimations(scene, this->lightHandler);
        this->endShadowMapRenderPass();
    }
    this->currentShadowMapCommandBuffer->end();


    // Begin regular command buffer
    this->currentCommandBuffer->beginOneTimeSubmit();

    // Render to HDR texture
    this->beginRenderPass();
        this->renderDefaultMeshes(scene);
        this->renderSkeletalAnimations(scene);
        this->renderParticles(scene);
    this->endRenderPass();

    this->currentCommandBuffer->end();


    // Downsample HDR texture
    for (uint32_t i = 1; i < this->postProcessHandler.getNumMipLevelsInUse(); ++i)
    {
        CommandBuffer& downsampleCommandBuffer =
            this->postProcessHandler.getDownsampleCommandBuffer(
                this->currentFrame,
                i
            );

        downsampleCommandBuffer.beginOneTimeSubmit();

        this->beginBloomDownUpsampleRenderPass(
            this->postProcessHandler.getDownsampleRenderPass(), 
            downsampleCommandBuffer, 
            i,
            false
        );
            this->renderBloomDownUpsample(
                downsampleCommandBuffer, 
                this->postProcessHandler.getDownsampleShaderInput(),
                this->postProcessHandler.getDownsamplePipeline(),
                i - 1
            );
        this->endBloomDownUpsampleRenderPass(downsampleCommandBuffer);

        downsampleCommandBuffer.end();
    }

    // Upsample HDR texture
    for (uint32_t i = this->postProcessHandler.getNumMipLevelsInUse() - 2; i >= 1; --i)
    {
        CommandBuffer& upsampleCommandBuffer =
            this->postProcessHandler.getUpsampleCommandBuffer(
                this->currentFrame,
                i
            );

        upsampleCommandBuffer.beginOneTimeSubmit();

        this->beginBloomDownUpsampleRenderPass(
            this->postProcessHandler.getUpsampleRenderPass(),
            upsampleCommandBuffer,
            i,
            true
        );
        this->renderBloomDownUpsample(
            upsampleCommandBuffer,
            this->postProcessHandler.getUpsampleShaderInput(),
            this->postProcessHandler.getUpsamplePipeline(),
            i + 1
        );
        this->endBloomDownUpsampleRenderPass(upsampleCommandBuffer);

        upsampleCommandBuffer.end();
    }


    // Begin swapchain command buffer
    this->currentSwapchainCommandBuffer->beginOneTimeSubmit();

    // Render to HDR texture to swapchain image, and UI/debug
    this->beginSwapchainRenderPass(imageIndex);
        this->renderToSwapchainImage();
        this->renderUI();
        this->renderDebugElements();
    this->endSwapchainRenderPass();

    // Imgui
    this->beginRenderpassImgui(imageIndex);
        this->renderImgui();
    this->endRenderpassImgui();

    this->currentSwapchainCommandBuffer->end();
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
    this->tracyContext.resize(this->commandBuffers.getNumCommandBuffers());
    for(size_t i = 0 ; i < this->commandBuffers.getNumCommandBuffers(); i++){
        
        this->tracyContext[i] = TracyVkContextCalibrated(
            this->physicalDevice.getVkPhysicalDevice(),
            this->getVkDevice(),             
            this->queueFamilies.getGraphicsQueue(),
            this->commandBuffers[i].getVkCommandBuffer(),
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
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    
    io.ConfigWindowsResizeFromEdges = true;
    
    ImGui::StyleColorsDark();
    this->window->initImgui();

    ImGui_ImplVulkan_InitInfo imguiInitInfo {};
    imguiInitInfo.Instance = this->instance.getVkInstance();
    imguiInitInfo.PhysicalDevice = this->physicalDevice.getVkPhysicalDevice();
    imguiInitInfo.Device = this->getVkDevice();
    imguiInitInfo.QueueFamily = this->queueFamilies.getGraphicsIndex();
    imguiInitInfo.Queue = this->queueFamilies.getGraphicsQueue();
    imguiInitInfo.PipelineCache = VK_NULL_HANDLE;
    imguiInitInfo.DescriptorPool = this->descriptorPoolImgui;
    imguiInitInfo.Subpass = 0; 
    imguiInitInfo.MinImageCount = this->swapchain.getNumMinimumImages();
    imguiInitInfo.ImageCount = this->swapchain.getNumImages();
    imguiInitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    imguiInitInfo.Allocator = nullptr;
    imguiInitInfo.CheckVkResultFn = checkVkResult;
    ImGui_ImplVulkan_Init(&imguiInitInfo, this->renderPassImgui.getVkRenderPass());

    // Imgui framebuffers
    std::vector<std::vector<vk::ImageView>> imguiFramebufferAttachments(this->swapchain.getNumImages());
    for (size_t i = 0; i < imguiFramebufferAttachments.size(); ++i)
    {
        imguiFramebufferAttachments[i].push_back(
            this->swapchain.getImageView(i)
        );
    }
    this->frameBuffersImgui.create(
        device,
        this->renderPassImgui,
        this->swapchain.getVkExtent(),
        imguiFramebufferAttachments
    );

    // Upload imgui font
    this->getVkDevice().resetCommandPool(this->commandPool);
    
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    const vk::CommandBuffer& firstCommandBuffer = 
        this->commandBuffers[0].getVkCommandBuffer();
    firstCommandBuffer.begin(beginInfo);
    
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