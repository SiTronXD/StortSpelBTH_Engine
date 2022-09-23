#include "VulkanRenderer.hpp"
#include "Utilities.hpp"
#include "assimp/Importer.hpp"
#include "vulkan/VulkanValidation.hpp"
#include "vulkan/VulkanDbg.hpp"
#include "../dev/defs.hpp"
#include "../dev/tracyHelper.hpp"
#include "../loaders/Configurator.hpp"
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

static void checkVkResult(VkResult err)
{
    if (err == 0)
        return;

    Log::error("Vulkan error from imgui: " + std::to_string(err));
}

using namespace vengine_helper::config;
int VulkanRenderer::init(Window* window, std::string&& windowName) 
{    
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    
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
        
        // Engine "specifics"

        // Shader inputs
        this->shaderInput.beginForInput(
            this->device,
            this->vma,
            MAX_FRAMES_IN_FLIGHT
        );
        this->shaderInput.addPushConstant(
            sizeof(ModelMatrix),
            vk::ShaderStageFlagBits::eVertex
        );
        this->createTextureSampler();
        this->shaderInput.addSampler();
        this->viewProjectionUB.createUniformBuffer(
            this->device,
            this->vma,
            sizeof(UboViewProjection),
            MAX_FRAMES_IN_FLIGHT
        );
        this->shaderInput.addUniformBuffer(this->viewProjectionUB);
        this->shaderInput.endForInput();

        this->pipeline.createPipeline(
            this->device, 
            this->shaderInput,
            this->renderPassBase
        );
        
        this->createSynchronisation();        

        this->updateUboProjection(); //TODO: Allow for more cameras! 
        this->updateUboView(
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
    tracy::GetProfiler().RequestShutdown(); //TODO: is this correct?    
#endif
    
    //Wait until no actions is run on device...
    this->device.waitIdle(); // Dont destroy semaphores before they are done
    
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

    for(auto & i : modelList)
    {
        i.destroyMeshModel();
    }

    this->getVkDevice().destroySampler(this->textureSampler);

    for(size_t i = 0; i < this->textureImages.size();i++)
    {
        this->getVkDevice().destroyImageView(this->textureImageViews[i]);
        this->getVkDevice().destroyImage(this->textureImages[i]);
        vmaFreeMemory(this->vma,this->textureImageMemory[i]);
    }

    this->viewProjectionUB.cleanup();

    for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        this->getVkDevice().destroySemaphore(this->renderFinished[i]);
        this->getVkDevice().destroySemaphore(this->imageAvailable[i]);
        this->getVkDevice().destroyFence(this->drawFences[i]);        
    }

    this->getVkDevice().destroyCommandPool(this->commandPool);
    
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

        std::vector<vk::CommandBufferSubmitInfo> commandBufferSubmitInfos
        {
            vk::CommandBufferSubmitInfo{
                this->commandBuffers.getCommandBuffer(this->currentFrame).
                    getVkCommandBuffer()
            }
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

    Buffer::createBuffer(
        {
            .bufferSize = imageSize, 
            .bufferUsageFlags = vk::BufferUsageFlagBits::eTransferSrc, 
            // .bufferProperties = vk::MemoryPropertyFlagBits::eHostVisible         // Staging buffer needs to be visible from HOST  (CPU), in order for modification
            //                     |   vk::MemoryPropertyFlagBits::eHostCoherent,   // not using cache...
            .bufferProperties = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                                | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .buffer = &imageStagingBuffer, 
            .bufferMemory = &imageStagingBufferMemory,
            .allocationInfo = &allocInfo,
            .vma = &this->vma
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
            .imageMemory = &texImageMemory
        },
        filename // Describing what image is being created, for debug purposes...
    );

    // - COPY THE DATA TO THE IMAGE -
    // Transition image to be in the DST, needed by the Copy Operation (Copy assumes/needs image Layout to be in vk::ImageLayout::eTransferDstOptimal state)
    Texture::transitionImageLayout(
        this->device, 
        this->queueFamilies.getGraphicsQueue(),
        this->commandPool, 
        texImage,                               // Image to transition the layout on
        vk::ImageLayout::eUndefined,              // Image Layout to transition the image from
        vk::ImageLayout::eTransferDstOptimal);  // Image Layout to transition the image to

    // Copy data to image
    Buffer::copyBufferToImage(
        this->device, 
        this->queueFamilies.getGraphicsQueue(),
        this->commandPool, 
        imageStagingBuffer, 
        texImage, 
        width, 
        height
    );

    // Transition iamge to be shader readable for shader usage
    Texture::transitionImageLayout(
        this->device, 
        this->queueFamilies.getGraphicsQueue(),
        this->commandPool, 
        texImage,
        vk::ImageLayout::eTransferDstOptimal,       // Image layout to transition the image from; this is the same as we transition the image too before we copied buffer!
        vk::ImageLayout::eShaderReadOnlyOptimal);  // Image Layout to transition the image to; in order for the Fragment Shader to read it!         

    // Add texture data to vector for reference 
    this->textureImages.push_back(texImage);
    this->textureImageMemory.push_back(texImageMemory);

    // Destroy and Free the staging buffer + staging buffer memroy
    this->getVkDevice().destroyBuffer(imageStagingBuffer);
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
    int descriptorLoc = this->shaderInput.addPossibleTexture(
        imageView, 
        this->textureSampler);

    // Return index of Texture Descriptor that was just created
    return descriptorLoc;
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
        this->commandPool, 
        scene, 
        matToTexture
    );

    // Create Model, add to list
    Model model = Model(
        &this->vma,
        this->physicalDevice.getVkPhysicalDevice(), 
        this->getVkDevice(), 
        this->queueFamilies.getGraphicsQueue(),
        this->commandPool,
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

void VulkanRenderer::createRenderPassBase() 
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    // Array of our subPasses    
    // std::array<vk::SubpassDescription2, 2> subPasses {};
    std::array<vk::SubpassDescription2, 1> subPasses{};

    // Color Attachment
    vk::AttachmentDescription2 colorAttachment {};
    colorAttachment.setFormat(this->swapchain.getVkFormat());
    colorAttachment.setSamples(vk::SampleCountFlagBits::e1);
    colorAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);      // When we start the renderpass, first thing to do is to clear since there is no values in it yet
    colorAttachment.setStoreOp(vk::AttachmentStoreOp::eStore); // How to store it after the RenderPass; We dont care! But we do care what happens after the first SubPass! (not handled here)
    colorAttachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
    colorAttachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
    colorAttachment.setInitialLayout(vk::ImageLayout::eUndefined);        // We dont care what the image layout is when we start. But we do care about what layout it is when it enter the first SubPass! (not handled here)
    colorAttachment.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal); //(!!) Should be the same value as it was after the subpass finishes (??)

    // Depth Attatchment
    vk::AttachmentDescription2 depthAttachment{};
    depthAttachment.setFormat(this->swapchain.getVkDepthFormat());
    depthAttachment.setSamples(vk::SampleCountFlagBits::e1);
    depthAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);      // Clear Buffer Whenever we try to load data into (i.e. clear before use it!)
    depthAttachment.setStoreOp(vk::AttachmentStoreOp::eDontCare); // Whenever it's used, we dont care what happens with the data... (we dont present it or anything)
    depthAttachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);  // Even though the Stencil i present, we dont plan to use it. so we dont care    
    depthAttachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);     // Even though the Stencil i present, we dont plan to use it. so we dont care
    depthAttachment.setInitialLayout(vk::ImageLayout::eUndefined);        // We don't care how the image layout is initially, so let it be undefined
    depthAttachment.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal); // Final layout should be Optimal for Depth Stencil attachment!

    // Color attachment reference
    vk::AttachmentReference2 colorAttachmentReference {};    
    colorAttachmentReference.setAttachment(uint32_t(0));            // Match the number/ID of the Attachment to the index of the FrameBuffer!
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
    poolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);   // Enables us to reset our CommandBuffers 
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
    uboViewProjection.projection  = glm::perspective(                               // View Angle in the y-axis
                            glm::radians(DEF<float>(CAM_FOV)),                               // View Angle in the y-axis
                            (float)this->swapchain.getWidth()/(float)swapchain.getHeight(),         // Setting up the Aspect Ratio
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
    renderPassBeginInfo.renderArea.setExtent(this->swapchain.getVkExtent());          // Size of region to run render pass on (starting at offset)
     
    static const vk::ClearColorValue clear_black(std::array<float,4> {0.F, 0.F, 0.F, 1.F});    
    static const vk::ClearColorValue clear_Plum(std::array<float,4> {221.F/256.0F, 160.F/256.0F, 221.F/256.0F, 1.0F});

    std::array<vk::ClearValue, 2> clearValues = 
    {
            vk::ClearValue(                              // of type VkClearColorValue 
                vk::ClearColorValue{clear_Plum}     // Clear Value for Attachment 0
            ),  
            vk::ClearValue(                              // Clear Value for Attachment 1
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
                
                // For every Mesh we have
                auto tView = scene->getSceneReg().view<Transform, MeshComponent>();
                tView.each([&](const Transform& transform, const MeshComponent& meshComponent)
                {
                    auto& currModel = modelList[meshComponent.meshID];

                    const glm::mat4& modelMatrix = transform.matrix;

                    // "Push" Constants to given Shader Stage Directly (using no Buffer...)
                    currentCommandBuffer.pushConstant(
                        this->shaderInput,
                        (void*) &modelMatrix
                    );

                    for(auto& modelPart : currModel.getModelParts())
                    {
                        // Bind vertex buffer
                        currentCommandBuffer.bindVertexBuffers2(modelPart.second.vertexBuffer);

                        // Bind index buffer
                        currentCommandBuffer.bindIndexBuffer(
                            modelPart.second.indexBuffer
                        );
                      
                        // We're going to bind Two descriptorSets! put them in array...
                        /*std::array<vk::DescriptorSet, 2> descriptorSetGroup
                        {
                            this->shaderInput.getDescriptorSet(this->currentFrame),                // Use the descriptor set for the Image                            
                            this->shaderInput.getSamplerDescriptorSet(modelPart.second.textureID)   // Use the Texture which the current mesh has
                        };*/

                        // Update for descriptors
                        this->shaderInput.setCurrentFrame(this->currentFrame);
                        this->shaderInput.setTexture(
                            0,
                            modelPart.second.textureID
                        );
                        currentCommandBuffer.bindShaderInput(
                            this->shaderInput
                        );

                        // Draw
                        currentCommandBuffer.drawIndexed(
                            modelPart.second.indexCount
                        );
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
    imguiInitInfo.PipelineCache = VK_NULL_HANDLE;  //TODO: Imgui Pipeline Should have its own Cache! 
    imguiInitInfo.DescriptorPool = this->descriptorPoolImgui;
    imguiInitInfo.Subpass = 0; 
    imguiInitInfo.MinImageCount = this->swapchain.getNumMinimumImages();
    imguiInitInfo.ImageCount = this->swapchain.getNumImages();
    imguiInitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT; //TODO: check correctness    
    imguiInitInfo.Allocator = nullptr;    //TODO: Can/should I pass in something VMA related here?
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
