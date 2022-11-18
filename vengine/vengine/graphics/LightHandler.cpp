#include "pch.h"
#include "LightHandler.hpp"
#include "../application/Scene.hpp"
#include "Texture.hpp"

void LightHandler::getWorldSpaceFrustumCorners(
    const glm::mat4& invViewProj,
    glm::vec4 outputCorners[])
{
    // Find frustum corners in world space
    for (uint32_t x = 0; x < 2; ++x)
    {
        for (uint32_t y = 0; y < 2; ++y)
        {
            for (uint32_t z = 0; z < 2; ++z)
            {
                uint32_t index = x * 2 * 2 + y * 2 + z;

                outputCorners[index] =
                    invViewProj * glm::vec4(
                        2.0f * float(x) - 1.0f,
                        2.0f * float(y) - 1.0f,
                        float(z),
                        1.0f
                    );
                outputCorners[index] /= outputCorners[index].w;
            }
        }
    }
}

void LightHandler::setLightFrustum(
    const glm::mat4& camProj,
    const glm::mat4& camView,
    glm::mat4& outputLightVP)
{
    // Find frustum corners in world space
    glm::vec4 frustumCorners[8];
    this->getWorldSpaceFrustumCorners(
        glm::inverse(camProj * camView),
        frustumCorners
    );

    glm::vec3 center(0.0f);
    for (uint32_t i = 0; i < 8; ++i)
    {
        center += glm::vec3(frustumCorners[i]);
    }
    center /= 8.0f;

    // Temporarily set VP to V
    glm::mat4 lightView =
        glm::lookAt(
            center,
            center + this->lightDir,
            this->lightUpDir
        );

    const auto firstViewSpaceCorner =
        lightView * frustumCorners[0];
    float minX = firstViewSpaceCorner.x;
    float maxX = firstViewSpaceCorner.x;
    float minY = firstViewSpaceCorner.y;
    float maxY = firstViewSpaceCorner.y;
    float minZ = firstViewSpaceCorner.z;
    float maxZ = firstViewSpaceCorner.z;
    for (uint32_t i = 1; i < 8; ++i)
    {
        const auto viewSpaceCorner = 
            lightView * frustumCorners[i];
        minX = std::min(minX, viewSpaceCorner.x);
        maxX = std::max(maxX, viewSpaceCorner.x);
        minY = std::min(minY, viewSpaceCorner.y);
        maxY = std::max(maxY, viewSpaceCorner.y);
        minZ = std::min(minZ, viewSpaceCorner.z);
        maxZ = std::max(maxZ, viewSpaceCorner.z);
    }

    // Scale depth
    if (minZ < 0.0f)
    {
        minZ *= this->cascadeDepthScale;
    }
    else
    {
        minZ /= this->cascadeDepthScale;
    }
    if (maxZ < 0.0f)
    {
        maxZ /= this->cascadeDepthScale;
    }
    else
    {
        maxZ *= this->cascadeDepthScale;
    }

    outputLightVP = 
        glm::orthoRH(minX, maxX, minY, maxY, minZ, maxZ) * 
        lightView;
}

LightHandler::LightHandler()
    : physicalDevice(nullptr),
    device(nullptr),
    vma(nullptr),
    resourceManager(nullptr),
    framesInFlight(0)
{ 
    this->cascadeSizes.resize(LightHandler::NUM_CASCADES - 1);
    this->cascadeNearPlanes.resize(LightHandler::NUM_CASCADES);

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
        vk::ImageUsageFlagBits::eSampled,
        resourceManager.addSampler(depthTextureSettings)
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
            this->shadowMapTexture.getImageView(i)
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
                this->cascadeSizes[i] = dirLightComp.cascadeSizes[i];
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

    // Create tightly fit light frustums
    for (uint32_t i = 0; i < LightHandler::NUM_CASCADES - 1; ++i)
    {
        this->shadowMapData.cascadeFarPlanes[i] =
            camData.farPlane * this->cascadeSizes[i];
    }
    this->shadowMapData.cascadeFarPlanes[LightHandler::NUM_CASCADES - 1]
        = camData.farPlane;

    // Create light matrices
    this->cascadeNearPlanes[0] = camData.nearPlane;
    for (uint32_t i = 1; i < LightHandler::NUM_CASCADES; ++i)
    {
        this->cascadeNearPlanes[i] =
            this->shadowMapData.cascadeFarPlanes[i - 1];
    }
    for (uint32_t i = 0; i < LightHandler::NUM_CASCADES; ++i)
    {
        this->setLightFrustum(
            glm::perspective(
                glm::radians(90.0f), 
                16.0f / 9.0f, 
                this->cascadeNearPlanes[i],
                this->shadowMapData.cascadeFarPlanes[i]
            ),
            camData.view,
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

void LightHandler::updateShadowPushConstant(
    CommandBuffer& currentShadowMapCommandBuffer,
    const glm::mat4& modelMatrix)
{
    // Update model matrix
    this->shadowPushConstantData.modelMatrix =
        modelMatrix;

    // Update push constant data
    currentShadowMapCommandBuffer.pushConstant(
        animShadowMapShaderInput,
        (void*) &this->shadowPushConstantData
    );

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