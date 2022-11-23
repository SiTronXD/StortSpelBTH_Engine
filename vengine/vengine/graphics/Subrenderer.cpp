#include "pch.h"
#include "VulkanRenderer.hpp"

void VulkanRenderer::beginShadowMapRenderPass(
    LightHandler& lightHandler,
    const uint32_t& shadowMapArraySlice)
{
    const std::array<vk::ClearValue, 1> clearValues =
    {
            vk::ClearValue(                         // Clear value for attachment 0
                vk::ClearDepthStencilValue(
                    1.F,    // depth
                    0       // stencil
                )
            )
    };

    const vk::Extent2D& shadowMapExtent = 
        lightHandler.getShadowMapExtent();

    // Information about how to begin a render pass
    vk::RenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.setRenderPass(lightHandler.getShadowMapRenderPass().getVkRenderPass());                      // Render pass to begin
    renderPassBeginInfo.renderArea.setOffset(vk::Offset2D(0, 0));                 // Start of render pass (in pixels...)
    renderPassBeginInfo.renderArea.setExtent(shadowMapExtent);      // Size of region to run render pass on (starting at offset)
    renderPassBeginInfo.setPClearValues(clearValues.data());
    renderPassBeginInfo.setClearValueCount(static_cast<uint32_t>(clearValues.size()));
    renderPassBeginInfo.setFramebuffer(lightHandler.getShadowMapFramebuffer(shadowMapArraySlice));

    // Begin Render Pass!    
    // vk::SubpassContents::eInline; all the render commands themselves will be primary render commands (i.e. will not use secondary commands buffers)
    vk::SubpassBeginInfoKHR subpassBeginInfo;
    subpassBeginInfo.setContents(vk::SubpassContents::eInline);
    this->currentShadowMapCommandBuffer->beginRenderPass2(
        renderPassBeginInfo,
        subpassBeginInfo
    );

    // Viewport
    vk::Viewport viewport{};
    viewport.x = 0.0f;
    viewport.y = (float) shadowMapExtent.height;
    viewport.width = (float) shadowMapExtent.width;
    viewport.height = -((float) shadowMapExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    this->currentShadowMapCommandBuffer->setViewport(viewport);

    // Scissor
    vk::Rect2D scissor{};
    scissor.offset = vk::Offset2D{ 0, 0 };
    scissor.extent = shadowMapExtent;
    this->currentShadowMapCommandBuffer->setScissor(scissor);

    // Update camera
    lightHandler.updateCamera(shadowMapArraySlice);
}

void VulkanRenderer::renderShadowMapDefaultMeshes(
    Scene* scene,
    LightHandler& lightHandler)
{
    ShaderInput& shadowMapShaderInput =
        lightHandler.getShadowMapShaderInput();

    // Bind Pipeline to be used in render pass
    this->currentShadowMapCommandBuffer->bindGraphicsPipeline(
        lightHandler.getShadowMapPipeline()
    );

    // Update for descriptors
    this->currentShadowMapCommandBuffer->bindShaderInputFrequency(
        shadowMapShaderInput,
        DescriptorFrequency::PER_FRAME
    );

    // For every non-animating mesh we have
    auto meshView = scene->getSceneReg().view<Transform, MeshComponent>(entt::exclude<AnimationComponent, Inactive>);
    meshView.each([&](
        const Transform& transform,
        const MeshComponent& meshComponent)
        {
            Mesh& currentMesh =
                this->resourceManager->getMesh(meshComponent.meshID);
            const std::vector<SubmeshData>& submeshes =
                currentMesh.getSubmeshData();

            // "Push" Constants to given Shader Stage Directly (using no Buffer...)
            lightHandler.updateDefaultShadowPushConstant(
                *this->currentShadowMapCommandBuffer,
                transform.getMatrix()
            );

            // Bind only vertex buffer containing positions
            this->currentShadowMapCommandBuffer->bindVertexBuffers2(
                currentMesh.getVertexBufferArray()
                    .getVertexBuffers()[0]
            );
            
            // Bind index buffer
            this->currentShadowMapCommandBuffer->bindIndexBuffer(
                currentMesh.getIndexBuffer()
            );

            // Update for descriptors
            this->currentShadowMapCommandBuffer->bindShaderInputFrequency(
                shadowMapShaderInput,
                DescriptorFrequency::PER_MESH
            );

            for (size_t i = 0; i < submeshes.size(); ++i)
            {
                const SubmeshData& currentSubmesh = submeshes[i];

                // Draw
                this->currentShadowMapCommandBuffer->drawIndexed(
                    currentSubmesh.numIndicies, 
                    1, 
                    currentSubmesh.startIndex
                );
            }
        }
    );
}

void VulkanRenderer::renderShadowMapSkeletalAnimations(
    Scene* scene,
    LightHandler& lightHandler)
{
    ShaderInput& animShadowMapShaderInput =
        lightHandler.getAnimShadowMapShaderInput();

    if (this->hasAnimations)
    {
        // Bind Pipeline to be used in render pass
        this->currentShadowMapCommandBuffer->bindGraphicsPipeline(
            lightHandler.getAnimShadowMapPipeline()
        );

        // Update for descriptors
        this->currentShadowMapCommandBuffer->bindShaderInputFrequency(
            animShadowMapShaderInput, 
            DescriptorFrequency::PER_FRAME
        );
    }

    // For every animating mesh we have
    auto animView = scene->getSceneReg().view<Transform, MeshComponent, AnimationComponent>(entt::exclude<Inactive>);
    animView.each(
        [&](const Transform& transform,
            const MeshComponent& meshComponent,
            const AnimationComponent& animationComponent)
        {
            Mesh& currentMesh =
                this->resourceManager->getMesh(meshComponent.meshID);
            MeshData& currentMeshData =
                currentMesh.getMeshData();
            const std::vector<SubmeshData>& submeshes =
                currentMesh.getSubmeshData();

            // Update transformations in storage buffer
            animShadowMapShaderInput.updateStorageBuffer(
                animationComponent.boneTransformsID,
                (void*) animationComponent.getBoneTransformsData()
            );
            animShadowMapShaderInput.setStorageBuffer(
                animationComponent.boneTransformsID
            );

            // "Push" Constants to given Shader Stage Directly (using no Buffer...)
            lightHandler.updateAnimatedShadowPushConstant(
                *this->currentShadowMapCommandBuffer,
                transform.getMatrix()
            );

            // Bind vertex buffer only containing positions, 
            // bone weights and bone indices
            this->bindVertexBufferOffsets = 
            {
                currentMesh.getVertexBufferArray().getVertexBufferOffsets()[0],
                currentMesh.getVertexBufferArray().getVertexBufferOffsets()[3],
                currentMesh.getVertexBufferArray().getVertexBufferOffsets()[4]
            };
            this->bindVertexBuffers = 
            {
                currentMesh.getVertexBufferArray().getVertexBuffers()[0],
                currentMesh.getVertexBufferArray().getVertexBuffers()[3],
                currentMesh.getVertexBufferArray().getVertexBuffers()[4]
            };
            this->currentShadowMapCommandBuffer->bindVertexBuffers2(
                this->bindVertexBufferOffsets,
                this->bindVertexBuffers
            );

            // Bind index buffer
            this->currentShadowMapCommandBuffer->bindIndexBuffer(
                currentMesh.getIndexBuffer()
            );

            // Update for descriptors
            this->currentShadowMapCommandBuffer->bindShaderInputFrequency(
                lightHandler.getAnimShadowMapShaderInput(),
                DescriptorFrequency::PER_MESH
            );

            for (size_t i = 0; i < submeshes.size(); ++i)
            {
                const SubmeshData& currentSubmesh = submeshes[i];

                // Draw
                this->currentShadowMapCommandBuffer->drawIndexed(
                    currentSubmesh.numIndicies, 
                    1, 
                    currentSubmesh.startIndex
                );
            }
        }
    );
}

void VulkanRenderer::endShadowMapRenderPass()
{
    // End render pass
    vk::SubpassEndInfo subpassEndInfo;
    this->currentShadowMapCommandBuffer->endRenderPass2(subpassEndInfo);
}

void VulkanRenderer::beginRenderPass()
{
    static const vk::ClearColorValue clearColor(
        // Sky color
        /*std::array<float, 4>
        {
            119.0f / 256.0f,
            172.0f / 256.0f,
            222.0f / 256.0f,
            1.0f
        }*/

        // Fog color
        std::array<float, 4>
        {
            0.8f,
            0.8f,
            0.8f,
            1.0f
        }
    );

    const std::array<vk::ClearValue, 2> clearValues =
    {
            vk::ClearValue(
                vk::ClearColorValue{ clearColor }     // Clear Value for Attachment 0
            ),
            vk::ClearValue(                         // Clear Value for Attachment 1
                vk::ClearDepthStencilValue(
                    1.F,    // depth
                    0       // stencil
                )
            )
    };

    const Texture& hdrRenderTexture = 
        this->postProcessHandler.getHdrRenderTexture();
    const vk::Extent2D extent(hdrRenderTexture.getWidth(), hdrRenderTexture.getHeight());

    // Information about how to begin a render pass
    vk::RenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.setRenderPass(this->renderPassBase.getVkRenderPass());                      // Render pass to begin
    renderPassBeginInfo.renderArea.setOffset(vk::Offset2D(0, 0));                 // Start of render pass (in pixels...)
    renderPassBeginInfo.renderArea.setExtent(extent);      // Size of region to run render pass on (starting at offset)
    renderPassBeginInfo.setPClearValues(clearValues.data());
    renderPassBeginInfo.setClearValueCount(static_cast<uint32_t>(clearValues.size()));
    renderPassBeginInfo.setFramebuffer(this->postProcessHandler.getRenderVkFramebuffer());

    // Begin Render Pass!    
    // vk::SubpassContents::eInline; all the render commands themselves will be primary render commands (i.e. will not use secondary commands buffers)
    vk::SubpassBeginInfoKHR subpassBeginInfo;
    subpassBeginInfo.setContents(vk::SubpassContents::eInline);
    this->currentCommandBuffer->beginRenderPass2(
        renderPassBeginInfo,
        subpassBeginInfo
    );

    // Viewport
    vk::Viewport viewport{};
    viewport.x = 0.0f;
    viewport.y = (float) extent.height;
    viewport.width = (float) extent.width;
    viewport.height = -((float) extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    this->currentCommandBuffer->setViewport(viewport);

    // Scissor
    vk::Rect2D scissor{};
    scissor.offset = vk::Offset2D{ 0, 0 };
    scissor.extent = extent;
    this->currentCommandBuffer->setScissor(scissor);
}

void VulkanRenderer::renderDefaultMeshes(
    Scene* scene)
{
    // Bind Pipeline to be used in render pass
    this->currentCommandBuffer->bindGraphicsPipeline(
        this->pipeline
    );

    // Update for descriptors
    this->currentCommandBuffer->bindShaderInputFrequency(
        this->shaderInput,
        DescriptorFrequency::PER_FRAME
    );

    // For every non-animating mesh we have
    auto meshView = scene->getSceneReg().view<Transform, MeshComponent>(entt::exclude<AnimationComponent, Inactive>);
    meshView.each([&](
        const Transform& transform,
        MeshComponent& meshComponent)
        {
            Mesh& currentMesh =
                this->resourceManager->getMesh(meshComponent.meshID);
            const std::vector<SubmeshData>& submeshes =
                currentMesh.getSubmeshData();
            const Material& firstSubmeshMaterial =
                this->getAppropriateMaterial(meshComponent, submeshes, 0);

            // "Push" Constants to given Shader Stage Directly (using no Buffer...)
            this->pushConstantData.modelMatrix = transform.getMatrix();
            this->pushConstantData.tintColor = firstSubmeshMaterial.tintColor;
            this->pushConstantData.emissionColor.r = firstSubmeshMaterial.emissionColor.r;
            this->pushConstantData.emissionColor.g = firstSubmeshMaterial.emissionColor.g;
            this->pushConstantData.emissionColor.b = firstSubmeshMaterial.emissionColor.b;
            this->currentCommandBuffer->pushConstant(
                this->shaderInput,
                (void*)&this->pushConstantData
            );

            // Bind vertex buffer
            this->currentCommandBuffer->bindVertexBuffers2(
                currentMesh.getVertexBufferArray(),
                this->currentFrame
            );

            // Bind index buffer
            this->currentCommandBuffer->bindIndexBuffer(
                currentMesh.getIndexBuffer()
            );

            // Update for descriptors
            this->currentCommandBuffer->bindShaderInputFrequency(
                this->shaderInput,
                DescriptorFrequency::PER_MESH
            );

            for (size_t i = 0; i < submeshes.size(); ++i)
            {
                const SubmeshData& currentSubmesh = submeshes[i];
                Material& material = 
                    this->getAppropriateMaterial(meshComponent, submeshes, i);

                // Create new material if found material is new
#if defined(_DEBUG) || defined(DEBUG)
                if (material.descriptorIndex >= ~0u)
                {
                    this->device.waitIdle();

                    FrequencyInputBindings diffuseTextureInputBinding{};
                    FrequencyInputBindings specularTextureInputBinding{};
                    diffuseTextureInputBinding.texture = &this->resourceManager->getTexture(material.diffuseTextureIndex);
                    specularTextureInputBinding.texture = &this->resourceManager->getTexture(material.specularTextureIndex);

                    // Add descriptor set
                    material.descriptorIndex =
                        this->shaderInput.addFrequencyInput(
                            {
                                diffuseTextureInputBinding,
                                specularTextureInputBinding
                            }
                    );

                    if (this->hasAnimations)
                    {
                        // Add one descriptor in animShaderInput for 
                        // each added descriptor in shaderInput
                        this->animShaderInput.addFrequencyInput(
                            {
                                diffuseTextureInputBinding,
                                specularTextureInputBinding
                            }
                        );
                    }
                }
#endif

                // Update for descriptors
                this->shaderInput.setFrequencyInput(
                    material.descriptorIndex
                );
                this->currentCommandBuffer->bindShaderInputFrequency(
                    this->shaderInput,
                    DescriptorFrequency::PER_DRAW_CALL
                );

                // Draw
                this->currentCommandBuffer->drawIndexed(
                    currentSubmesh.numIndicies, 1, currentSubmesh.startIndex
                );
            }
        }
    );
}

void VulkanRenderer::renderSkeletalAnimations(Scene* scene)
{
    if (this->hasAnimations)
    {
        // Bind Pipeline to be used in render pass
        this->currentCommandBuffer->bindGraphicsPipeline(
            this->animPipeline
        );

        // Update for descriptors
        this->currentCommandBuffer->bindShaderInputFrequency(
            this->animShaderInput, DescriptorFrequency::PER_FRAME
        );
    }

    // For every animating mesh we have
    auto animView = scene->getSceneReg().view<Transform, MeshComponent, AnimationComponent>(entt::exclude<Inactive>);
    animView.each(
        [&](const Transform& transform,
            const MeshComponent& meshComponent,
            const AnimationComponent& animationComponent)
        {
            Mesh& currentMesh =
                this->resourceManager->getMesh(meshComponent.meshID);
            MeshData& currentMeshData =
                currentMesh.getMeshData();
            const std::vector<SubmeshData>& submeshes =
                currentMesh.getSubmeshData();
            const Material& firstSubmeshMaterial =
                this->getAppropriateMaterial(meshComponent, submeshes, 0);

            // Update transformations in storage buffer
            this->animShaderInput.updateStorageBuffer(
                animationComponent.boneTransformsID,
                (void*) animationComponent.getBoneTransformsData()
            );
            this->animShaderInput.setStorageBuffer(
                animationComponent.boneTransformsID
            );

            // "Push" Constants to given Shader Stage Directly (using no Buffer...)
            this->pushConstantData.modelMatrix = transform.getMatrix();
            this->pushConstantData.tintColor = firstSubmeshMaterial.tintColor;
            this->pushConstantData.emissionColor.r = firstSubmeshMaterial.emissionColor.r;
            this->pushConstantData.emissionColor.g = firstSubmeshMaterial.emissionColor.g;
            this->pushConstantData.emissionColor.b = firstSubmeshMaterial.emissionColor.b;
            this->currentCommandBuffer->pushConstant(
                this->animShaderInput,
                (void*)&this->pushConstantData
            );

            // Bind vertex buffer
            this->currentCommandBuffer->bindVertexBuffers2(
                currentMesh.getVertexBufferArray(),
                this->currentFrame
            );

            // Bind index buffer
            this->currentCommandBuffer->bindIndexBuffer(
                currentMesh.getIndexBuffer()
            );

            // Update for descriptors
            this->currentCommandBuffer->bindShaderInputFrequency(
                this->animShaderInput,
                DescriptorFrequency::PER_MESH
            );

            for (size_t i = 0; i < submeshes.size(); ++i)
            {
                const SubmeshData& currentSubmesh = submeshes[i];

                // Update for descriptors
                this->animShaderInput.setFrequencyInput(
                    this->getAppropriateMaterial(meshComponent, submeshes, i)
                    .descriptorIndex
                );
                this->currentCommandBuffer->bindShaderInputFrequency(
                    this->animShaderInput,
                    DescriptorFrequency::PER_DRAW_CALL
                );

                // Draw
                this->currentCommandBuffer->drawIndexed(
                    currentSubmesh.numIndicies, 1, currentSubmesh.startIndex
                );
            }
        }
    );
}

void VulkanRenderer::renderUI()
{
    // UI pipeline
    this->currentSwapchainCommandBuffer->bindGraphicsPipeline(
        this->uiRenderer->getPipeline()
    );

    // UI storage buffer
    this->uiRenderer->getShaderInput().setStorageBuffer(
        this->uiRenderer->getStorageBufferID()
    );
    this->currentSwapchainCommandBuffer->bindShaderInputFrequency(
        this->uiRenderer->getShaderInput(),
        DescriptorFrequency::PER_MESH
    );

    // UI update storage buffer
    this->uiRenderer->getShaderInput().updateStorageBuffer(
        this->uiRenderer->getStorageBufferID(),
        this->uiRenderer->getUiElementData().data()
    );

    // One draw call for all ui elements with the same texture
    const std::vector<UIDrawCallData>& drawCallData =
        this->uiRenderer->getUiDrawCallData();
    for (size_t i = 0; i < drawCallData.size(); ++i)
    {
        // UI texture
        this->uiRenderer->getShaderInput().setFrequencyInput(
            this->resourceManager->getTexture(drawCallData[i].textureIndex)
            .getDescriptorIndex()
        );
        this->currentSwapchainCommandBuffer->bindShaderInputFrequency(
            this->uiRenderer->getShaderInput(),
            DescriptorFrequency::PER_DRAW_CALL
        );

        // UI draw
        this->currentSwapchainCommandBuffer->draw(
            drawCallData[i].numVertices,
            1,
            drawCallData[i].startVertex
        );
    }

    this->uiRenderer->resetRender();
}

void VulkanRenderer::renderDebugElements()
{
    // ---------- Lines ----------

    // Pipeline
    this->currentSwapchainCommandBuffer->bindGraphicsPipeline(
        this->debugRenderer->getLinePipeline()
    );

    // Bind shader input per frame
    this->currentSwapchainCommandBuffer->bindShaderInputFrequency(
        this->debugRenderer->getLineShaderInput(),
        DescriptorFrequency::PER_FRAME
    );

    // Bind vertex buffer
    this->currentSwapchainCommandBuffer->bindVertexBuffers2(
        this->debugRenderer->getLineVertexBufferArray(),
        this->currentFrame
    );

    // Draw
    this->currentSwapchainCommandBuffer->draw(this->debugRenderer->getNumVertices());

    // ---------- Meshes ----------

    // Pipeline
    this->currentSwapchainCommandBuffer->bindGraphicsPipeline(
        this->debugRenderer->getMeshPipeline()
    );

    // Bind shader input per frame
    this->currentSwapchainCommandBuffer->bindShaderInputFrequency(
        this->debugRenderer->getMeshShaderInput(),
        DescriptorFrequency::PER_FRAME
    );

    // Loop through meshes and render them
    const std::vector<DebugMeshDrawCallData>& debugDrawCallData
        = this->debugRenderer->getMeshDrawCallData();
    for (size_t i = 0; i < debugDrawCallData.size(); ++i)
    {
        const Mesh& currentMesh =
            this->resourceManager->getMesh(debugDrawCallData[i].meshID);
        const SubmeshData& currentSubmesh =
            currentMesh.getSubmeshData()[0];

        // Push constant
        this->currentSwapchainCommandBuffer->pushConstant(
            this->debugRenderer->getMeshShaderInput(),
            (void*)&debugDrawCallData[i].pushConstantData
        );

        // Bind vertex buffer
        this->currentSwapchainCommandBuffer->bindVertexBuffers2(
            currentMesh.getVertexBufferArray(),
            this->currentFrame
        );

        // Bind index buffer
        this->currentSwapchainCommandBuffer->bindIndexBuffer(
            currentMesh.getIndexBuffer()
        );

        // Draw
        this->currentSwapchainCommandBuffer->drawIndexed(
            currentSubmesh.numIndicies,
            1,
            currentSubmesh.startIndex
        );
    }

    this->debugRenderer->resetRender();
}

void VulkanRenderer::endRenderPass()
{
    // End Render Pass!
    vk::SubpassEndInfo subpassEndInfo;
    this->currentCommandBuffer->endRenderPass2(subpassEndInfo);
}

void VulkanRenderer::beginBloomDownUpsampleRenderPass(
    const RenderPass& renderPass,
    CommandBuffer& commandBuffer,
    const uint32_t& writeMipIndex)
{
    static const vk::ClearColorValue clearColor(
        // Fog color
        std::array<float, 4>
        {
            0.0f,
            0.0f,
            0.0f,
            1.0f
        }
    );

    const std::array<vk::ClearValue, 1> clearValues =
    {
            vk::ClearValue(
                vk::ClearColorValue{ clearColor }
            )
    };

    const vk::Extent2D& hdrTextureExtent =
        this->postProcessHandler.getMipExtent(writeMipIndex);

    // Information about how to begin a render pass
    vk::RenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.setRenderPass(renderPass.getVkRenderPass());                      // Render pass to begin
    renderPassBeginInfo.renderArea.setOffset(vk::Offset2D(0, 0));                 // Start of render pass (in pixels...)
    renderPassBeginInfo.renderArea.setExtent(hdrTextureExtent);      // Size of region to run render pass on (starting at offset)
    renderPassBeginInfo.setPClearValues(clearValues.data());
    renderPassBeginInfo.setClearValueCount(static_cast<uint32_t>(clearValues.size()));
    renderPassBeginInfo.setFramebuffer(this->postProcessHandler.getMipVkFramebuffer(writeMipIndex));

    // Begin Render Pass!    
    // vk::SubpassContents::eInline; all the render commands themselves will be primary render commands (i.e. will not use secondary commands buffers)
    vk::SubpassBeginInfoKHR subpassBeginInfo;
    subpassBeginInfo.setContents(vk::SubpassContents::eInline);
    commandBuffer.beginRenderPass2(
        renderPassBeginInfo,
        subpassBeginInfo
    );

    // Viewport
    vk::Viewport viewport{};
    viewport.x = 0.0f;
    viewport.y = (float) hdrTextureExtent.height;
    viewport.width = (float) hdrTextureExtent.width;
    viewport.height = -((float) hdrTextureExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    commandBuffer.setViewport(viewport);

    // Scissor
    vk::Rect2D scissor{};
    scissor.offset = vk::Offset2D{ 0, 0 };
    scissor.extent = hdrTextureExtent;
    commandBuffer.setScissor(scissor);
}

void VulkanRenderer::renderBloomDownUpsample(
    CommandBuffer& commandBuffer,
    ShaderInput& shaderInput,
    const Pipeline& pipeline,
    const uint32_t& readMipIndex)
{
    // Bind Pipeline to be used in render pass
    commandBuffer.bindGraphicsPipeline(
        pipeline
    );

    // Update resolution
    const vk::Extent2D& mipExtent = this->postProcessHandler.getMipExtent(readMipIndex);
    ResolutionPushConstantData& pushConstantData =
        this->postProcessHandler.getResolutionData();
    pushConstantData.resolution.x = (float) mipExtent.width;
    pushConstantData.resolution.y = (float) mipExtent.height;
    commandBuffer.pushConstant(
        shaderInput,
        (void*) &pushConstantData
    );

    // Bind texture for descriptors
    shaderInput.setFrequencyInput(
        this->postProcessHandler.getMipDescriptorIndex(readMipIndex)
    );
    commandBuffer.bindShaderInputFrequency(
        shaderInput,
        DescriptorFrequency::PER_DRAW_CALL
    );

    // Draw fullscreen quad
    commandBuffer.draw(6);
}

void VulkanRenderer::endBloomDownUpsampleRenderPass(CommandBuffer& commandBuffer)
{
    // End Render Pass!
    vk::SubpassEndInfo subpassEndInfo;
    commandBuffer.endRenderPass2(subpassEndInfo);
}

void VulkanRenderer::beginSwapchainRenderPass(
    const uint32_t& imageIndex)
{
    static const vk::ClearColorValue clearColor(
        // Fog color
        std::array<float, 4>
        {
            0.0f,
            0.0f,
            0.0f,
            1.0f
        }
    );

    const std::array<vk::ClearValue, 2> clearValues =
    {
            vk::ClearValue(
                vk::ClearColorValue{ clearColor }     // Clear Value for Attachment 0
            ),
            vk::ClearValue(                         // Clear Value for Attachment 1
                vk::ClearDepthStencilValue(
                    1.F,    // depth
                    0       // stencil
                )
            )
    };

    const vk::Extent2D& swapchainExtent =
        this->swapchain.getVkExtent();

    // Information about how to begin a render pass
    vk::RenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.setRenderPass(this->renderPassSwapchain.getVkRenderPass());                      // Render pass to begin
    renderPassBeginInfo.renderArea.setOffset(vk::Offset2D(0, 0));                 // Start of render pass (in pixels...)
    renderPassBeginInfo.renderArea.setExtent(swapchainExtent);      // Size of region to run render pass on (starting at offset)
    renderPassBeginInfo.setPClearValues(clearValues.data());
    renderPassBeginInfo.setClearValueCount(static_cast<uint32_t>(clearValues.size()));
    renderPassBeginInfo.setFramebuffer(this->swapchain.getVkFramebuffer(imageIndex));

    // Begin Render Pass!    
    // vk::SubpassContents::eInline; all the render commands themselves will be primary render commands (i.e. will not use secondary commands buffers)
    vk::SubpassBeginInfoKHR subpassBeginInfo;
    subpassBeginInfo.setContents(vk::SubpassContents::eInline);
    this->currentSwapchainCommandBuffer->beginRenderPass2(
        renderPassBeginInfo,
        subpassBeginInfo
    );

    // Viewport
    vk::Viewport viewport{};
    viewport.x = 0.0f;
    viewport.y = (float) swapchainExtent.height;
    viewport.width = (float) swapchainExtent.width;
    viewport.height = -((float) swapchainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    this->currentSwapchainCommandBuffer->setViewport(viewport);

    // Scissor
    vk::Rect2D scissor{};
    scissor.offset = vk::Offset2D{ 0, 0 };
    scissor.extent = swapchainExtent;
    this->currentSwapchainCommandBuffer->setScissor(scissor);
}

void VulkanRenderer::renderToSwapchainImage()
{
    // Bind Pipeline to be used in render pass
    this->currentSwapchainCommandBuffer->bindGraphicsPipeline(
        this->swapchainPipeline
    );

    // Update for descriptors
    this->currentSwapchainCommandBuffer->bindShaderInputFrequency(
        this->swapchainShaderInput,
        DescriptorFrequency::PER_FRAME
    );

    // Bind HDR texture for descriptors
    this->swapchainShaderInput.setFrequencyInput(
        this->hdrRenderTextureDescriptorIndex
    );
    this->currentSwapchainCommandBuffer->bindShaderInputFrequency(
        this->swapchainShaderInput,
        DescriptorFrequency::PER_DRAW_CALL
    );

    // Draw fullscreen quad
    this->currentSwapchainCommandBuffer->draw(6);
}

void VulkanRenderer::endSwapchainRenderPass()
{
    // End Render Pass!
    vk::SubpassEndInfo subpassEndInfo;
    this->currentSwapchainCommandBuffer->endRenderPass2(subpassEndInfo);
}

void VulkanRenderer::beginRenderpassImgui(
    const uint32_t& imageIndex)
{
    // Begin render pass
    vk::RenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.setRenderPass(this->renderPassImgui.getVkRenderPass());
    renderPassBeginInfo.renderArea.setExtent(this->swapchain.getVkExtent());
    renderPassBeginInfo.renderArea.setOffset(vk::Offset2D(0, 0));
    renderPassBeginInfo.setFramebuffer(this->frameBuffersImgui[imageIndex]);
    renderPassBeginInfo.setClearValueCount(uint32_t(0));

    vk::SubpassBeginInfo subpassBeginInfo{};
    subpassBeginInfo.setContents(vk::SubpassContents::eInline);
    this->currentSwapchainCommandBuffer->beginRenderPass2(
        renderPassBeginInfo,
        subpassBeginInfo
    );

    // Viewport
    vk::Viewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)this->swapchain.getWidth();
    viewport.height = (float)swapchain.getHeight();
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    this->currentSwapchainCommandBuffer->setViewport(viewport);

    // Scissor
    vk::Rect2D scissor{};
    scissor.offset = vk::Offset2D{ 0, 0 };
    scissor.extent = this->swapchain.getVkExtent();
    this->currentSwapchainCommandBuffer->setScissor(scissor);
}

void VulkanRenderer::renderImgui()
{
    ImGui_ImplVulkan_RenderDrawData(
        ImGui::GetDrawData(),
        this->currentSwapchainCommandBuffer->getVkCommandBuffer()
    );
}

void VulkanRenderer::endRenderpassImgui()
{
    vk::SubpassEndInfo imguiSubpassEndInfo;
    this->currentSwapchainCommandBuffer->endRenderPass2(imguiSubpassEndInfo);
}