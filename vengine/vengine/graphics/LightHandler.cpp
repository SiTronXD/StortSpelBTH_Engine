#include "pch.h"
#include "LightHandler.hpp"
#include "../application/Scene.hpp"

LightHandler::LightHandler()
    : physicalDevice(nullptr),
    device(nullptr),
    vma(nullptr)
{ }

void LightHandler::init(
    PhysicalDevice& physicalDevice, 
    Device& device,
    VmaAllocator& vma,
    vk::CommandPool& commandPool,
    ResourceManager& resourceManager,
    const uint32_t& framesInFlight)
{
    this->physicalDevice = &physicalDevice;
    this->device = &device;
    this->vma = &vma;
    this->resourceManager = &resourceManager;
    this->framesInFlight = framesInFlight;

    this->shadowMapExtent = vk::Extent2D(
        LightHandler::SHADOW_MAP_SIZE,
        LightHandler::SHADOW_MAP_SIZE
    );
    this->shadowMapData.shadowMapSize.x = (float)this->shadowMapExtent.width;
    this->shadowMapData.shadowMapSize.y = (float)this->shadowMapExtent.height;

    // Sampling settings 
    TextureSettings depthTextureSettings{};
    depthTextureSettings.samplerSettings.filterMode =
        vk::Filter::eNearest;

    // Texture
    this->shadowMapTexture.createAsDepthTexture(
        *this->physicalDevice,
        *this->device,
        *this->vma,
        this->shadowMapExtent.width,
        this->shadowMapExtent.height,
        vk::ImageUsageFlagBits::eSampled,
        resourceManager.addSampler(depthTextureSettings)
    );

    // Render pass
    this->shadowMapRenderPass.createRenderPassShadowMap(
        *this->device,
        this->shadowMapTexture
    );

    // Framebuffer
    this->shadowMapFramebuffer.create(
        *this->device,
        this->shadowMapRenderPass,
        this->shadowMapExtent,
        {
            {
                this->shadowMapTexture.getImageView()
            }
        }
    );

    // Command buffer array
    this->shadowMapCommandBuffers.createCommandBuffers(
        *this->device,
        commandPool,
        framesInFlight
    );

    // Preset shadow map data
    this->shadowMapData.projection =
        glm::orthoRH(
            -50.0f, 50.0f,
            -50.0f, 50.0f,
            0.1f, 400.0f
        );
}

void LightHandler::initForScene(
    Scene* scene,
    const bool& oldHasAnimations,
    const bool& hasAnimations)
{
    // Try to cleanup before creating new objects
    this->shadowMapShaderInput.cleanup();
    this->shadowMapPipeline.cleanup();
    if (oldHasAnimations) // (hasAnimations from previous scene)
    {
        this->animShadowMapShaderInput.cleanup();
        this->animShadowMapPipeline.cleanup();
    }

    // Bind only positions when rendering shadow map
    VertexStreams shadowMapVertexStream{};
    shadowMapVertexStream.positions.resize(1);

    this->shadowMapShaderInput.beginForInput(
        *this->physicalDevice,
        *this->device,
        *this->vma,
        *this->resourceManager,
        this->framesInFlight
    );
    this->shadowMapShaderInput.addPushConstant(
        sizeof(PushConstantData),
        vk::ShaderStageFlagBits::eVertex
    );
    this->shadowMapViewProjectionUB =
        this->shadowMapShaderInput.addUniformBuffer(
            sizeof(CameraBufferData),
            vk::ShaderStageFlagBits::eVertex,
            DescriptorFrequency::PER_FRAME
        );
    this->shadowMapShaderInput.endForInput();
    this->shadowMapPipeline.createPipeline(
        *this->device,
        this->shadowMapShaderInput,
        this->shadowMapRenderPass,
        shadowMapVertexStream,
        "shadowMap.vert.spv",
        ""
    );

    // Make sure animated meshes actually exists
    if (hasAnimations)
    {
        VertexStreams animShadowMapStream{};
        animShadowMapStream.positions.resize(1);
        animShadowMapStream.boneWeights.resize(1);
        animShadowMapStream.boneIndices.resize(1);

        this->animShadowMapShaderInput.beginForInput(
            *this->physicalDevice,
            *this->device,
            *this->vma,
            *this->resourceManager,
            this->framesInFlight
        );
        this->animShadowMapShaderInput.addPushConstant(
            sizeof(PushConstantData),
            vk::ShaderStageFlagBits::eVertex
        );
        this->animShadowMapShaderInput.setNumShaderStorageBuffers(1);

        // Add shader inputs for animations
        auto tView =
            scene->getSceneReg().view<Transform, MeshComponent, AnimationComponent>();
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
                this->animShadowMapShaderInput.addStorageBuffer(
                    numAnimationBones * sizeof(glm::mat4),
                    vk::ShaderStageFlagBits::eVertex,
                    DescriptorFrequency::PER_MESH
                );
            }
        );
        this->animShadowMapViewProjectionUB =
            this->animShadowMapShaderInput.addUniformBuffer(
                sizeof(CameraBufferData),
                vk::ShaderStageFlagBits::eVertex,
                DescriptorFrequency::PER_FRAME
            );
        this->animShadowMapShaderInput.endForInput();
        this->animShadowMapPipeline.createPipeline(
            *this->device,
            this->animShadowMapShaderInput,
            this->shadowMapRenderPass,
            animShadowMapStream,
            "shadowMapAnim.vert.spv",
            ""
        );
    }
}

void LightHandler::updateLightBuffers(
    Scene* scene,
    ShaderInput& shaderInput,
    ShaderInput& animShaderInput,
    const UniformBufferID& allLightsInfoUB,
    const UniformBufferID& animAllLightsInfoUB,
    const StorageBufferID& lightBufferSB,
    const StorageBufferID& animLightBufferSB,
    const bool& hasAnimations,
    const glm::vec3& camPosition,
    const uint32_t& currentFrame)
{
    this->lightBuffer.clear();

    // Info about all lights in the shader
    AllLightsInfo lightsInfo{};

    // Loop through all ambient lights in scene
    auto ambientLightView = scene->getSceneReg().view<AmbientLight>(entt::exclude<Inactive>);
    ambientLightView.each([&](
        const AmbientLight& ambientLightComp)
        {
            // Create point light data
            LightBufferData lightData{};
            lightData.color = glm::vec4(ambientLightComp.color, 1.0f);

            // Add to list
            this->lightBuffer.push_back(lightData);

            // Increment end index
            lightsInfo.ambientLightsEndIndex++;
        }
    );

    // Loop through all directional lights in the scene
    lightsInfo.directionalLightsEndIndex = lightsInfo.ambientLightsEndIndex;
    auto directionalLightView = scene->getSceneReg().view<DirectionalLight>(entt::exclude<Inactive>);
    directionalLightView.each([&](
        const DirectionalLight& directionalLightComp)
        {
            // Create point light data
            LightBufferData lightData{};
            lightData.direction =
                glm::vec4(glm::normalize(directionalLightComp.direction), 1.0f);
            lightData.color =
                glm::vec4(directionalLightComp.color, 1.0f);

            // Add to list
            this->lightBuffer.push_back(lightData);

            // Increment end index
            lightsInfo.directionalLightsEndIndex++;
        }
    );

    // Loop through all point lights in scene
    lightsInfo.pointLightsEndIndex = lightsInfo.directionalLightsEndIndex;
    auto pointLightView = scene->getSceneReg().view<Transform, PointLight>(entt::exclude<Inactive>);
    pointLightView.each([&](
        Transform& transform,
        const PointLight& pointLightComp)
        {
            // Create point light data
            LightBufferData lightData{};
            lightData.position = glm::vec4(
                transform.position +
                transform.getRotationMatrix() * pointLightComp.positionOffset,
                1.0f);
            lightData.color = glm::vec4(pointLightComp.color, 1.0f);

            // Add to list
            this->lightBuffer.push_back(lightData);

            // Increment end index
            lightsInfo.pointLightsEndIndex++;
        }
    );

    // Loop through all spotlights in scene
    lightsInfo.spotlightsEndIndex = lightsInfo.pointLightsEndIndex;
    auto spotlightView = scene->getSceneReg().view<Transform, Spotlight>(entt::exclude<Inactive>);
    spotlightView.each([&](
        Transform& transform,
        const Spotlight& spotlightComp)
        {
            const glm::mat3 rotMat = transform.getRotationMatrix();

            // Create point light data
            LightBufferData lightData{};
            lightData.position = glm::vec4(
                transform.position +
                rotMat * spotlightComp.positionOffset,
                1.0f
            );
            lightData.direction = glm::vec4(
                glm::normalize(rotMat * spotlightComp.direction),
                std::cos(glm::radians(spotlightComp.angle * 0.5f))
            );
            lightData.color = glm::vec4(spotlightComp.color, 1.0f);

            // Add to list
            this->lightBuffer.push_back(lightData);

            // Increment end index
            lightsInfo.spotlightsEndIndex++;
        }
    );

    // Update storage buffer containing lights
    if (this->lightBuffer.size() > 0)
    {
        shaderInput.updateStorageBuffer(lightBufferSB, (void*)this->lightBuffer.data());
        if (hasAnimations)
        {
            animShaderInput.updateStorageBuffer(animLightBufferSB, (void*)this->lightBuffer.data());
        }
    }

    // Truncate indices to not overshoot max
    lightsInfo.ambientLightsEndIndex = std::min(
        lightsInfo.ambientLightsEndIndex,
        LightHandler::MAX_NUM_LIGHTS);
    lightsInfo.directionalLightsEndIndex = std::min(
        lightsInfo.directionalLightsEndIndex,
        LightHandler::MAX_NUM_LIGHTS);
    lightsInfo.pointLightsEndIndex = std::min(
        lightsInfo.pointLightsEndIndex,
        LightHandler::MAX_NUM_LIGHTS);
    lightsInfo.spotlightsEndIndex = std::min(
        lightsInfo.spotlightsEndIndex,
        LightHandler::MAX_NUM_LIGHTS);

#ifdef _CONSOLE
    if (this->lightBuffer.size() > LightHandler::MAX_NUM_LIGHTS)
    {
        Log::warning("The number of lights is larger than the maximum allowed number. Truncates " +
            std::to_string(this->lightBuffer.size()) + " lights to " +
            std::to_string(MAX_NUM_LIGHTS));
    }
#endif

    // Update all lights info buffer
    shaderInput.updateUniformBuffer(
        allLightsInfoUB,
        (void*)&lightsInfo
    );
    if (hasAnimations)
    {
        animShaderInput.updateUniformBuffer(
            animAllLightsInfoUB,
            (void*)&lightsInfo
        );
    }

    // Update shadow map view matrix
    auto dirLightView = scene->getSceneReg().view<DirectionalLight>(entt::exclude<Inactive>);
    dirLightView.each([&](
        DirectionalLight& dirLightComp)
        {
            dirLightComp.direction =
                glm::normalize(dirLightComp.direction);

            glm::vec3 lightPos =
                camPosition - dirLightComp.direction * 200.0f;

            glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
            if (std::abs(glm::dot(worldUp, dirLightComp.direction)) >= 0.95f)
                worldUp = glm::vec3(0.0f, 0.0f, 1.0f);

            this->shadowMapData.view = glm::lookAt(
                lightPos,
                lightPos + dirLightComp.direction,
                worldUp
            );
        }
    );

    // Shadow map shader input
    this->shadowMapShaderInput.setCurrentFrame(currentFrame);
    this->shadowMapShaderInput.updateUniformBuffer(
        this->shadowMapViewProjectionUB,
        (void*)&shadowMapData
    );

    // Anim shadow map shader input
    if (hasAnimations)
    {
        this->animShadowMapShaderInput.setCurrentFrame(
            currentFrame
        );
        this->animShadowMapShaderInput.updateUniformBuffer(
            this->animShadowMapViewProjectionUB,
            (void*)&shadowMapData
        );
    }
}

void LightHandler::cleanup()
{
    this->shadowMapTexture.cleanup();
    this->shadowMapRenderPass.cleanup();
    this->shadowMapFramebuffer.cleanup();
    
    this->shadowMapShaderInput.cleanup();
    this->shadowMapPipeline.cleanup();

    this->animShadowMapShaderInput.cleanup();
    this->animShadowMapPipeline.cleanup();
}