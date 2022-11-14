#include "pch.h"
#include "VulkanRenderer.hpp"

void VulkanRenderer::beginRenderpass(
    const uint32_t& imageIndex)
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

    // Information about how to begin a render pass
    vk::RenderPassBeginInfo renderPassBeginInfo;
    renderPassBeginInfo.setRenderPass(this->renderPassBase);                      // Render pass to begin
    renderPassBeginInfo.renderArea.setOffset(vk::Offset2D(0, 0));                 // Start of render pass (in pixels...)
    renderPassBeginInfo.renderArea.setExtent(this->swapchain.getVkExtent());      // Size of region to run render pass on (starting at offset)
    renderPassBeginInfo.setPClearValues(clearValues.data());
    renderPassBeginInfo.setClearValueCount(static_cast<uint32_t>(clearValues.size()));
    renderPassBeginInfo.setFramebuffer(this->swapchain.getVkFramebuffer(imageIndex));

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
    viewport.y = (float)swapchain.getHeight();
    viewport.width = (float)this->swapchain.getWidth();
    viewport.height = -((float)swapchain.getHeight());
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    this->currentCommandBuffer->setViewport(viewport);

    // Scissor
    vk::Rect2D scissor{};
    scissor.offset = vk::Offset2D{ 0, 0 };
    scissor.extent = this->swapchain.getVkExtent();
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
        const MeshComponent& meshComponent)
        {
            Mesh& currentMesh =
                this->resourceManager->getMesh(meshComponent.meshID);
            const std::vector<SubmeshData>& submeshes =
                currentMesh.getSubmeshData();

            // "Push" Constants to given Shader Stage Directly (using no Buffer...)
            this->pushConstantData.modelMatrix = transform.getMatrix();
            this->pushConstantData.tintColor =
                this->getAppropriateMaterial(meshComponent, submeshes, 0).tintColor;
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

                // Update for descriptors
                this->shaderInput.setFrequencyInput(
                    this->getAppropriateMaterial(meshComponent, submeshes, i)
                    .descriptorIndex
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

            // Get bone transformations
            const std::vector<glm::mat4>& boneTransforms =
                currentMesh.getBoneTransforms(animationComponent.timer,
                    animationComponent.animationIndex);

            // Update transformations in storage buffer
            this->animShaderInput.updateStorageBuffer(
                animationComponent.boneTransformsID,
                (void*)&boneTransforms[0]
            );
            this->animShaderInput.setStorageBuffer(
                animationComponent.boneTransformsID
            );

            // "Push" Constants to given Shader Stage Directly (using no Buffer...)
            this->pushConstantData.modelMatrix = transform.getMatrix();
            this->pushConstantData.tintColor =
                this->getAppropriateMaterial(meshComponent, submeshes, 0).tintColor;
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
    this->currentCommandBuffer->bindGraphicsPipeline(
        this->uiRenderer->getPipeline()
    );

    // UI storage buffer
    this->uiRenderer->getShaderInput().setStorageBuffer(
        this->uiRenderer->getStorageBufferID()
    );
    this->currentCommandBuffer->bindShaderInputFrequency(
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
        this->currentCommandBuffer->bindShaderInputFrequency(
            this->uiRenderer->getShaderInput(),
            DescriptorFrequency::PER_DRAW_CALL
        );

        // UI draw
        this->currentCommandBuffer->draw(
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
    this->currentCommandBuffer->bindGraphicsPipeline(
        this->debugRenderer->getLinePipeline()
    );

    // Bind shader input per frame
    this->currentCommandBuffer->bindShaderInputFrequency(
        this->debugRenderer->getLineShaderInput(),
        DescriptorFrequency::PER_FRAME
    );

    // Bind vertex buffer
    this->currentCommandBuffer->bindVertexBuffers2(
        this->debugRenderer->getLineVertexBufferArray(),
        this->currentFrame
    );

    // Draw
    this->currentCommandBuffer->draw(this->debugRenderer->getNumVertices());

    // ---------- Meshes ----------

    // Pipeline
    this->currentCommandBuffer->bindGraphicsPipeline(
        this->debugRenderer->getMeshPipeline()
    );

    // Bind shader input per frame
    this->currentCommandBuffer->bindShaderInputFrequency(
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
        this->currentCommandBuffer->pushConstant(
            this->debugRenderer->getMeshShaderInput(),
            (void*)&debugDrawCallData[i].pushConstantData
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

        // Draw
        this->currentCommandBuffer->drawIndexed(
            currentSubmesh.numIndicies,
            1,
            currentSubmesh.startIndex
        );
    }

    this->debugRenderer->resetRender();
}

void VulkanRenderer::endRenderpass()
{
    // End Render Pass!
    vk::SubpassEndInfo subpassEndInfo;
    this->currentCommandBuffer->endRenderPass2(subpassEndInfo);
}

void VulkanRenderer::beginRenderpassImgui(
    const uint32_t& imageIndex)
{
    // Begin second render pass
    vk::RenderPassBeginInfo renderPassBeginInfo{};
    vk::SubpassBeginInfo subpassBeginInfo;
    subpassBeginInfo.setContents(vk::SubpassContents::eInline);
    renderPassBeginInfo.setRenderPass(this->renderPassImgui);
    renderPassBeginInfo.renderArea.setExtent(this->swapchain.getVkExtent());
    renderPassBeginInfo.renderArea.setOffset(vk::Offset2D(0, 0));
    renderPassBeginInfo.setFramebuffer(this->frameBuffersImgui[imageIndex]);
    renderPassBeginInfo.setClearValueCount(uint32_t(0));

    this->currentCommandBuffer->beginRenderPass2(
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
    this->currentCommandBuffer->setViewport(viewport);

    // Scissor
    vk::Rect2D scissor{};
    scissor.offset = vk::Offset2D{ 0, 0 };
    scissor.extent = this->swapchain.getVkExtent();
    this->currentCommandBuffer->setScissor(scissor);
}

void VulkanRenderer::renderImgui()
{
    ImGui_ImplVulkan_RenderDrawData(
        ImGui::GetDrawData(),
        this->currentCommandBuffer->getVkCommandBuffer()
    );
}

void VulkanRenderer::endRenderpassImgui()
{
    vk::SubpassEndInfo imguiSubpassEndInfo;
    this->currentCommandBuffer->endRenderPass2(imguiSubpassEndInfo);
}