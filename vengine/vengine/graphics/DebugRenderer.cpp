#include "pch.h"
#include "DebugRenderer.hpp"
#include "vulkan/ShaderStructs.hpp"
#include "../dev/Log.hpp"
#include "../resource_management/ResourceManager.hpp"
#include "../components/AnimationComponent.hpp"
#include "../components/MeshComponent.hpp"
#include "../components/Transform.hpp"
#include "../components/PointLight.hpp"
#include "../components/Spotlight.hpp"
#include "../VengineMath.hpp"
#include "../application/SceneHandler.hpp"

void DebugRenderer::prepareGPU(const uint32_t& currentFrame)
{
    // Update vertex buffers on gpu
    this->lineVertexBuffers.cpuUpdate(
        0,
        currentFrame,
        this->lineVertexStreams.positions
    );
    this->lineVertexBuffers.cpuUpdate(
        1,
        currentFrame,
        this->lineVertexStreams.colors
    );
}

void DebugRenderer::resetRender()
{
    // Clear for batch of render commands next frame
    this->numVertices = 0;

    this->meshDrawData.clear();
}

glm::vec3 DebugRenderer::toneMappingACES(const glm::vec3& x)
{
    const float a = 2.51f;
    const float b = 0.03f;
    const float c = 2.43f;
    const float d = 0.59f;
    const float e = 0.14f;
    return
        glm::vec3(
            ((x.x * (a * x.x + b)) / (x.x * (c * x.x + d) + e)),
            ((x.y * (a * x.y + b)) / (x.y * (c * x.y + d) + e)),
            ((x.z * (a * x.z + b)) / (x.z * (c * x.z + d) + e))
        );
}

DebugRenderer::DebugRenderer()
    : physicalDevice(nullptr),
    device(nullptr),
    vma(nullptr),
    resourceManager(nullptr),
    renderPass(nullptr),
    framesInFlight(0),
    numVertices(0),
    sceneHandler(nullptr)
{

}

void DebugRenderer::setSceneHandler(SceneHandler* sceneHandler)
{
    this->sceneHandler = sceneHandler;
}

void DebugRenderer::create(
    PhysicalDevice& physicalDevice,
    Device& device,
    VmaAllocator& vma,
    ResourceManager& resourceManager,
    RenderPass& renderPass,
    vk::Queue& transferQueue,
    vk::CommandPool& transferCommandPool,
    const uint32_t& framesInFlight)
{
    this->physicalDevice = &physicalDevice;
    this->device = &device;
    this->vma = &vma;
    this->resourceManager = &resourceManager;
    this->renderPass = &renderPass;
    this->transferQueue = &transferQueue;
    this->transferCommandPool = &transferCommandPool;
    this->framesInFlight = framesInFlight;
}

void DebugRenderer::initForScene()
{
    // Cleanup shader input if possible
    this->cleanup();

    // Line vertex buffers
    this->lineVertexBuffers.createForCpu(
        *this->device,
        *this->vma,
        *this->transferQueue,
        *this->transferCommandPool,
        this->framesInFlight
    );
    this->lineVertexStreams.positions.resize(START_NUM_MAX_ELEMENTS);
    this->lineVertexStreams.colors.resize(START_NUM_MAX_ELEMENTS);
    this->lineVertexBuffers.addCpuVertexBuffer(this->lineVertexStreams.positions);
    this->lineVertexBuffers.addCpuVertexBuffer(this->lineVertexStreams.colors);

    // Line shader input
    this->lineShaderInput.beginForInput(
        *this->physicalDevice,
        *this->device,
        *this->vma,
        *this->resourceManager,
        this->framesInFlight);
    this->lineViewProjectionUB =
        this->lineShaderInput.addUniformBuffer(
            sizeof(CameraBufferData),
            vk::ShaderStageFlagBits::eVertex,
            DescriptorFrequency::PER_FRAME
        );
    this->lineShaderInput.endForInput();

    // Line pipeline
    this->linePipeline.createPipeline(
        *this->device,
        this->lineShaderInput,
        *this->renderPass,
        this->lineVertexStreams,
        "dbgLine.vert.spv",
        "dbg.frag.spv",
        false,
        false,
        true,
        vk::PrimitiveTopology::eLineList
    );

    // Mesh shader input
    this->meshShaderInput.beginForInput(
        *this->physicalDevice,
        *this->device,
        *this->vma,
        *this->resourceManager,
        this->framesInFlight);
    this->meshViewProjectionUB = 
        this->meshShaderInput.addUniformBuffer(
            sizeof(CameraBufferData),
            vk::ShaderStageFlagBits::eVertex,
            DescriptorFrequency::PER_FRAME
        );
    this->meshShaderInput.addPushConstant(
        sizeof(DebugMeshPushConstantData), 
        vk::ShaderStageFlagBits::eVertex
    );
    this->meshShaderInput.endForInput();

    // Mesh pipeline
    VertexStreams meshVertexStreams{};
    meshVertexStreams.positions.resize(1);
    this->meshPipeline.createPipeline(
        *this->device,
        this->meshShaderInput,
        *this->renderPass,
        meshVertexStreams,
        "dbgMesh.vert.spv",
        "dbg.frag.spv",
        false,
        true,
        false
    );

    // Add meshes for debug rendering
    this->upperHemisphereMeshID = 
        this->resourceManager->addMesh("vengine_assets/models/upperHemisphere.obj");
    this->lowerHemisphereMeshID =
        this->resourceManager->addMesh("vengine_assets/models/lowerHemisphere.obj");
    this->capsuleBodyMeshID =
        this->resourceManager->addMesh("vengine_assets/models/capsuleBody.obj");
    this->sphereMeshID = 
        this->resourceManager->addMesh("vengine_assets/models/icoSphere.obj");
    this->boxMeshID = 
        this->resourceManager->addMesh("vengine_assets/models/box.obj");
}

void DebugRenderer::cleanup()
{
    // Mesh rendering
    this->meshPipeline.cleanup();
    this->meshShaderInput.cleanup();

    // Line rendering
    this->lineVertexBuffers.cleanup();
    this->linePipeline.cleanup();
    this->lineShaderInput.cleanup();
}

void DebugRenderer::renderLine(
    const glm::vec3& pos0, 
    const glm::vec3& pos1,
    const glm::vec3& color)
{
#if defined(_CONSOLE) // Debug/Release, but not distribution

    if (this->numVertices >= START_NUM_MAX_ELEMENTS)
    {
        Log::error("Reached maximum number of rendered debug elements.");
        return;
    }

    // Add positions/colors
    this->lineVertexStreams.positions[this->numVertices] = pos0;
    this->lineVertexStreams.colors[this->numVertices] = color;
    this->numVertices++;
    this->lineVertexStreams.positions[this->numVertices] = pos1;
    this->lineVertexStreams.colors[this->numVertices] = color;
    this->numVertices++;

#endif
}

void DebugRenderer::renderPointLight(const Entity& pointLightEntity)
{
    Scene* scene = this->sceneHandler->getScene();

    if (!scene->hasComponents<PointLight>(pointLightEntity))
    {
        Log::warning("Entity ID " + std::to_string(pointLightEntity) + " does not have a point light. Please remove the call to DebugRenderer::renderPointLight() for this entity.");
        return;
    }

    Transform& transform = scene->getComponent<Transform>(pointLightEntity);
    PointLight& pointLight = scene->getComponent<PointLight>(pointLightEntity);
    transform.updateMatrix();
    
    // Transform position
    glm::vec3 position = transform.position + 
        transform.getRotationMatrix() * pointLight.positionOffset;

    // Render point light as box
    this->renderBox(
        position, 
        transform.rotation, 
        glm::vec3(0.3f), 
        this->toneMappingACES(pointLight.color)
    );
}

void DebugRenderer::renderSpotlight(const Entity& spotlightEntity)
{
    Scene* scene = this->sceneHandler->getScene();

    if (!scene->hasComponents<Spotlight>(spotlightEntity))
    {
        Log::warning("Entity ID " + std::to_string(spotlightEntity) + " does not have a spotlight. Please remove the call to DebugRenderer::renderSpotlight() for this entity.");
        return;
    }

    Transform& transform = scene->getComponent<Transform>(spotlightEntity);
    Spotlight& spotlight = scene->getComponent<Spotlight>(spotlightEntity);
    transform.updateMatrix();

    // Transform
    glm::vec3 position = transform.position +
        transform.getRotationMatrix() * spotlight.positionOffset;
    glm::vec3 direction = transform.getRotationMatrix() * spotlight.direction;

    glm::vec3 color = this->toneMappingACES(spotlight.color);

    // Render point light position as sphere
    this->renderSphere(
        position,
        0.1f,
        color
    );

    float len = 15.0f;

    // Render direction as line
    /*this->renderLine(
        position, 
        position + direction * len,
        color
    );*/

    // World up
    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    if (abs(dot(worldUp, direction)) >= 0.95f)
        worldUp = glm::vec3(1.0f, 0.0f, 0.0f);

    // Render tangents to cone
    glm::vec3 rightVec = normalize(glm::cross(direction, worldUp));
    glm::vec3 upVec = normalize(glm::cross(rightVec, direction));
    float perpLen = std::sin(glm::radians(spotlight.angle) * 0.5f) * len;
    this->renderLine(
        position,
        position + direction * len + rightVec * perpLen,
        color
    );
    this->renderLine(
        position,
        position + direction * len - rightVec * perpLen,
        color
    );
    this->renderLine(
        position,
        position + direction * len + upVec * perpLen,
        color
    );
    this->renderLine(
        position,
        position + direction * len - upVec * perpLen,
        color
    );
}

void DebugRenderer::renderParticleSystemCone(
    const Entity& particleSystemEntity,
    const glm::vec3& color)
{
    Scene* scene = this->sceneHandler->getScene();
    Transform& transform = scene->getComponent<Transform>(particleSystemEntity);
    ParticleSystem& particleSystem = scene->getComponent<ParticleSystem>(particleSystemEntity);

    // Different transformations
    transform.updateMatrix();
    const glm::mat3& transformRot =
        transform.getRotationMatrix();
    const glm::vec3 coneDir = glm::normalize(
        transformRot *
        particleSystem.coneSpawnVolume.localDirection
    );
    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    if (glm::abs(glm::dot(worldUp, coneDir)) >= 0.95f)
        worldUp = glm::vec3(1.0f, 0.0f, 0.0f);
    const glm::vec3 coneNormal = 
        glm::normalize(glm::cross(worldUp, coneDir));
    const glm::mat3 coneRotMat =
        glm::mat3(
            coneNormal,
            glm::cross(coneNormal, coneDir),
            coneDir
        );

    // Render lines as cone tangents
    const uint32_t numLines = 16;
    glm::vec3 positions0[numLines];
    glm::vec3 positions1[numLines];
    for (uint32_t i = 0; i < numLines; ++i)
    {
        const glm::mat4 tempRot = glm::rotate(
            glm::mat4(1.0f),
            float(i) / numLines * SMath::PI * 2.0f,
            glm::vec3(0.0f, 0.0f, 1.0f)
        );

        // Base position
        glm::vec3 diskOffset =
            glm::vec3(0.0f, 1.0f, 0.0f) * particleSystem.coneSpawnVolume.diskRadius;
        diskOffset =
            tempRot * glm::vec4(diskOffset, 0.0f);
        positions0[i] =
            transform.position + 
            transformRot *
            particleSystem.coneSpawnVolume.localPosition +
            coneRotMat * diskOffset;

        // Cone tangent direction
        const float strength = 5.0f;
        const float tanTheta =
            std::tan(
                glm::radians(
                    std::clamp(
                        particleSystem.coneSpawnVolume.coneAngle * 0.5f,
                        0.0f,
                        89.0f
                    )
                )
            );
        positions1[i] = 
            positions0[i] + 
            (coneDir + tanTheta * coneRotMat * glm::vec3(tempRot * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f))) * strength;

        // Render line
        this->renderLine(positions0[i], positions1[i], color);
    }

    // Stitch together lines in disk
    for (uint32_t i = 0; i < numLines/2; ++i)
    {
        this->renderLine(positions0[i], positions0[(i + numLines/2) % numLines], color);
    }
    for (uint32_t i = 0; i < numLines; ++i)
    {
        this->renderLine(positions0[i], positions0[(i + 1) % numLines], color);
        this->renderLine(positions1[i], positions1[(i + 1) % numLines], color);
    }
}

void DebugRenderer::renderSphere(
    const glm::vec3& position,
    const float& radius,
    const glm::vec3& color)
{
#if defined(_CONSOLE) // Debug/Release, but not distribution

    // Add draw call data
    DebugMeshDrawCallData drawCallData{};

    drawCallData.meshID = this->sphereMeshID;
    drawCallData.pushConstantData.transform = 
        glm::translate(glm::mat4(1.0f), position) * 
        glm::scale(glm::mat4(1.0f), glm::vec3(radius));
    drawCallData.pushConstantData.color = glm::vec4(color, 1.0f);

    this->meshDrawData.push_back(drawCallData);

#endif
}

void DebugRenderer::renderBox(
    const glm::vec3& position,
    const glm::vec3& eulerRotation,
    const glm::vec3& halfExtents,
    const glm::vec3& color)
{
#if defined(_CONSOLE) // Debug/Release, but not distribution

    // Add draw call data
    DebugMeshDrawCallData drawCallData{};

    glm::mat4 rotationMatrix = SMath::rotateEuler(eulerRotation);

    drawCallData.meshID = this->boxMeshID;
    drawCallData.pushConstantData.transform = 
        glm::translate(glm::mat4(1.0f), position) *
        rotationMatrix * 
        glm::scale(glm::mat4(1.0f), halfExtents);
    drawCallData.pushConstantData.color = glm::vec4(color, 1.0f);

    this->meshDrawData.push_back(drawCallData);

#endif
}

void DebugRenderer::renderCapsule(
    const glm::vec3& position,
    const glm::vec3& eulerRotation,
    const float& height,
    const float& radius,
    const glm::vec3& color)
{
#if defined(_CONSOLE) // Debug/Release, but not distribution

    glm::mat4 rotationMatrix = SMath::rotateEuler(eulerRotation);
    glm::mat4 hemisphereScaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(radius));

    // Upper hemisphere draw call data
    DebugMeshDrawCallData upperHemisphereData{};
    upperHemisphereData.meshID = this->upperHemisphereMeshID;
    upperHemisphereData.pushConstantData.transform =
        glm::translate(glm::mat4(1.0f), position) *
        rotationMatrix *
        glm::translate(glm::mat4(1.0f), 
            glm::vec3(0.0f, (height - radius * 2.0f) * 0.5f, 0.0f)) *
        hemisphereScaleMatrix;
    upperHemisphereData.pushConstantData.color = glm::vec4(color, 1.0f);

    // Lower hemisphere draw call data
    DebugMeshDrawCallData lowerHemisphereData{};
    lowerHemisphereData.meshID = this->lowerHemisphereMeshID;
    lowerHemisphereData.pushConstantData.transform =
        glm::translate(glm::mat4(1.0f), position) *
        rotationMatrix *
        glm::translate(glm::mat4(1.0f),
            glm::vec3(0.0f, -(height - radius * 2.0f) * 0.5f, 0.0f)) *
        hemisphereScaleMatrix;
    lowerHemisphereData.pushConstantData.color = glm::vec4(color, 1.0f);

    // Capsule body draw call data
    DebugMeshDrawCallData capsuleBodyData{};
    capsuleBodyData.meshID = this->capsuleBodyMeshID;
    capsuleBodyData.pushConstantData.transform =
        glm::translate(glm::mat4(1.0f), position) *
        rotationMatrix * 
        glm::scale(glm::mat4(1.0f), glm::vec3(radius, height - radius * 2.0f, radius));
    capsuleBodyData.pushConstantData.color = glm::vec4(color, 1.0f);

    // Add draw call data
    this->meshDrawData.push_back(upperHemisphereData);
    this->meshDrawData.push_back(lowerHemisphereData);
    this->meshDrawData.push_back(capsuleBodyData);

#endif
}

void DebugRenderer::renderSkeleton(
    const Entity& entity,
    const glm::vec3& color)
{
#if defined(_CONSOLE) // Debug/Release, but not distribution

    Scene* scene = this->sceneHandler->getScene();

    // Make sure entity is valid
    if (!scene->entityValid(entity))
    {
        Log::error("This entity is not valid when rendering skeleton.");
        return;
    }

    // Make sure entity has mesh component
    if (!scene->hasComponents<MeshComponent>(entity))
    {
        Log::error("This entity does not have a mesh component required for rendering the skeleton.");
        return;
    }

    // Info
    Mesh& mesh = this->resourceManager->getMesh(
        scene->getComponent<MeshComponent>(entity).meshID);
    MeshData& meshData = mesh.getMeshData();
    const glm::mat4& modelMat = scene->getComponent<Transform>(entity).getMatrix();

    // Loop through bones and render them as lines
    size_t numBones = meshData.bones.size();
    glm::vec3 parentBonePos(0.0f);
    glm::vec3 bonePos(0.0f);
    glm::mat4* parentBoneTransform = nullptr;
    glm::mat4* boneTransform = nullptr;
    for (size_t i = 1; i < numBones; ++i)
    {
        if (meshData.bones[i].parentIndex < 0)
            continue;

        // Transforms
        parentBoneTransform = &meshData.bones[meshData.bones[i].parentIndex].boneMatrix;
        boneTransform = &meshData.bones[i].boneMatrix;

        // Positions
        parentBonePos = SMath::extractTranslation(*parentBoneTransform);
        bonePos = SMath::extractTranslation(*boneTransform);

        // Transform by model matrix
        parentBonePos = modelMat * glm::vec4(parentBonePos, 1.0f);
        bonePos = modelMat * glm::vec4(bonePos, 1.0f);

        // Render line
        this->renderLine(parentBonePos, bonePos, color);
    }

#endif
}

void DebugRenderer::renderParticleSystemProperties(
    Scene* scene,
    const Entity& particleSystemEntity,
    const std::string& name)
{
#ifdef _CONSOLE
    ImGui::Begin("Particle System");
    {
        Transform& partSysTran = scene->getComponent<Transform>(particleSystemEntity);
        ParticleSystem& partSys = scene->getComponent<ParticleSystem>(particleSystemEntity);
        ImGui::SliderFloat3((name + " entity position : ").c_str(), &partSysTran.position[0], -100.0f, 100.0f);
        ImGui::SliderFloat3((name + " entity rotation: ").c_str(), &partSysTran.rotation[0], -180.0f, 180.0f);
        ImGui::SliderFloat4((name + " start color: ").c_str(), &partSys.startColor[0], 0.0f, 1.0f);
        ImGui::SliderFloat4((name + " end color: ").c_str(), &partSys.endColor[0], 0.0f, 1.0f);
        ImGui::SliderFloat3((name + " cone pos: ").c_str(), &partSys.coneSpawnVolume.localPosition[0], -5.0f, 5.0f);
        ImGui::SliderFloat3((name + " cone dir: ").c_str(), &partSys.coneSpawnVolume.localDirection[0], -1.0f, 1.0f);
        ImGui::SliderFloat((name + " disk radius: ").c_str(), &partSys.coneSpawnVolume.diskRadius, 0.0f, 10.0f);
        ImGui::SliderFloat((name + " cone angle: ").c_str(), &partSys.coneSpawnVolume.coneAngle, 0.0f, 180.0f);
        ImGui::SliderFloat((name + " velocity strength: ").c_str(), &partSys.velocityStrength, 0.0f, 20.0f);
        ImGui::SliderFloat((name + " spawn rate: ").c_str(), &partSys.spawnRate, 0.0f, 1.0f);
        ImGui::Checkbox((name + " spawn: ").c_str(), &partSys.spawn);

        static bool renderPS = true;
        ImGui::Checkbox((name + " render cone: ").c_str(), &renderPS);
        if (renderPS)
            this->renderParticleSystemCone(particleSystemEntity);
    }
    ImGui::End();
#endif
}