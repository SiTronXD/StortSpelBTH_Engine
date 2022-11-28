#include "pch.h"
#include "LightHandler.hpp"
#include "../../application/Scene.hpp"
#include "../Texture.hpp"

const uint32_t LightHandler::MAX_NUM_LIGHTS   = 16;
const uint32_t LightHandler::SHADOW_MAP_SIZE  = 1024 * 3;
const uint32_t LightHandler::NUM_CASCADES     = 3;

void LightHandler::setLightFrustum(
    const float& cascadeSize,
    glm::mat4& outputLightVP)
{
    float xSize = cascadeSize;
    float ySize = cascadeSize;
    float zSize = cascadeSize * this->cascadeDepthScale;

    // Combine view and unique projection
    outputLightVP = 
        glm::orthoRH(
            -xSize, xSize, 
            -ySize, ySize, 
            -zSize, zSize
        ) * this->lightViewMat;
}

LightHandler::LightHandler()
    : physicalDevice(nullptr),
    device(nullptr),
    vma(nullptr),
    resourceManager(nullptr),
    framesInFlight(0),
    lightViewMat(1.0f)
{ 
    this->cascadeSizes.resize(LightHandler::NUM_CASCADES);

    this->shadowMapData.cascadeSettings.x = 
        LightHandler::NUM_CASCADES;
}

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
        LightHandler::NUM_CASCADES,
        resourceManager.addSampler(depthTextureSettings),
        vk::ImageUsageFlagBits::eSampled
    );

    // Render pass
    this->shadowMapRenderPass.createRenderPassShadowMap(
        *this->device,
        this->shadowMapTexture
    );

    // Framebuffer
    std::vector<std::vector<vk::ImageView>> shadowMapImageViews(LightHandler::NUM_CASCADES);
    for (size_t i = 0; i < LightHandler::NUM_CASCADES; ++i)
    {
        shadowMapImageViews[i] =
        {
            this->shadowMapTexture.getLayerImageView(i)
        };
    }
    this->shadowMapFramebuffer.create(
        *this->device,
        this->shadowMapRenderPass,
        this->shadowMapExtent,
        shadowMapImageViews
    );

    // Command buffer array
    this->shadowMapCommandBuffers.createCommandBuffers(
        *this->device,
        commandPool,
        framesInFlight
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
        sizeof(ShadowPushConstantData),
        vk::ShaderStageFlagBits::eVertex
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
            sizeof(ShadowPushConstantData),
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
    const Camera& camData,
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

    // Update shadow map settings
    auto dirLightView = scene->getSceneReg().view<DirectionalLight>(entt::exclude<Inactive>);
    dirLightView.each([&](
        DirectionalLight& dirLightComp)
        {
            // Normalize direction
            dirLightComp.direction =
                glm::normalize(dirLightComp.direction);
            this->lightDir = dirLightComp.direction;

            // Cascade settings
            for (size_t i = 0; i < this->cascadeSizes.size(); ++i)
            {
                this->cascadeSizes[i] = dirLightComp.cascadeSizes[i];// / camData.aspectRatio * (16.0f / 9.0f);
            }
            this->cascadeDepthScale = dirLightComp.cascadeDepthScale;
            this->shadowMapData.cascadeSettings.y = dirLightComp.cascadeVisualization ? 1.0f : 0.0f;

            // Biases
            this->shadowMapData.shadowMapMinBias = dirLightComp.shadowMapMinBias;
            this->shadowMapData.shadowMapAngleBias = dirLightComp.shadowMapAngleBias;
        }
    );

    // Update up direction
    this->lightUpDir = glm::vec3(0.0f, 1.0f, 0.0f);
    if (std::abs(glm::dot(this->lightUpDir, this->lightDir)) >= 0.95f)
        this->lightUpDir = glm::vec3(0.0f, 0.0f, 1.0f);

    // Update view matrix
    this->lightViewMat = 
        glm::lookAt(
            camPosition - this->lightDir,
            camPosition,
            this->lightUpDir
        );

    // Create light VP matrices
    for (uint32_t i = 0; i < LightHandler::NUM_CASCADES; ++i)
    {
        this->setLightFrustum(
            this->cascadeSizes[i],
            this->shadowMapData.viewProjection[i]
        );
    }

    // Shadow map shader input
    this->shadowMapShaderInput.setCurrentFrame(currentFrame);

    // Anim shadow map shader input
    if (hasAnimations)
    {
        this->animShadowMapShaderInput.setCurrentFrame(
            currentFrame
        );
    }
}

void LightHandler::updateCamera(
    const uint32_t& arraySliceCameraIndex)
{
    // Update projection matrix
    this->shadowPushConstantData.viewProjectionMatrix =
        this->shadowMapData.viewProjection[arraySliceCameraIndex];
}

void LightHandler::updateDefaultShadowPushConstant(
    CommandBuffer& currentShadowMapCommandBuffer,
    const glm::mat4& modelMatrix)
{
    // Update model matrix
    this->shadowPushConstantData.modelMatrix =
        modelMatrix;

    // Update push constant data
    currentShadowMapCommandBuffer.pushConstant(
        this->shadowMapShaderInput,
        (void*)&this->shadowPushConstantData
    );
}

void LightHandler::updateAnimatedShadowPushConstant(
    CommandBuffer& currentShadowMapCommandBuffer,
    const glm::mat4& modelMatrix)
{
    // Update model matrix
    this->shadowPushConstantData.modelMatrix =
        modelMatrix;

    // Update push constant data
    currentShadowMapCommandBuffer.pushConstant(
        this->animShadowMapShaderInput,
        (void*)&this->shadowPushConstantData
    );
}

void LightHandler::cleanup(const bool& hasAnimations)
{
    this->shadowMapTexture.cleanup();
    this->shadowMapRenderPass.cleanup();
    this->shadowMapFramebuffer.cleanup();
    
    this->shadowMapShaderInput.cleanup();
    this->shadowMapPipeline.cleanup();

    if (hasAnimations)
    {
        this->animShadowMapShaderInput.cleanup();
        this->animShadowMapPipeline.cleanup();
    }
}