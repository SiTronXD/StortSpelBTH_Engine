#include "DebugRenderer.hpp"
#include "vulkan/UniformBufferStructs.hpp"
#include "../dev/Log.hpp"
#include "../resource_management/ResourceManager.hpp"
#include "../components/AnimationComponent.hpp"
#include "../components/Transform.hpp"
#include "../VengineMath.hpp"

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

DebugRenderer::DebugRenderer()
    : physicalDevice(nullptr),
    device(nullptr),
    vma(nullptr),
    resourceManager(nullptr),
    renderPass(nullptr),
    framesInFlight(0),
    numVertices(0)
{

}

void DebugRenderer::create(
    PhysicalDevice& physicalDevice,
    Device& device,
    VmaAllocator& vma,
    ResourceManager& resourceManager,
    vk::RenderPass& renderPass,
    vk::Queue& transferQueue,
    vk::CommandPool& transferCommandPool,
    const uint32_t& framesInFlight)
{
    this->physicalDevice = &physicalDevice;
    this->device = &device;
    this->vma = &vma;
    this->resourceManager = &resourceManager;
    this->renderPass = &renderPass;
    this->transferQueue = &transferQueue,
    this->transferCommandPool = &transferCommandPool,
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
        this->lineShaderInput.addUniformBuffer(sizeof(UboViewProjection));
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
        this->meshShaderInput.addUniformBuffer(sizeof(UboViewProjection));
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
}

void DebugRenderer::renderSphere(
    const glm::vec3& position,
    const float& radius,
    const glm::vec3& color)
{
    // Add draw call data
    DebugMeshDrawCallData drawCallData{};

    drawCallData.meshID = this->sphereMeshID;
    drawCallData.pushConstantData.transform = 
        glm::translate(glm::mat4(1.0f), position) * 
        glm::scale(glm::mat4(1.0f), glm::vec3(radius));
    drawCallData.pushConstantData.color = glm::vec4(color, 1.0f);

    this->meshDrawData.push_back(drawCallData);
}

void DebugRenderer::renderBox(
    const glm::vec3& position,
    const glm::vec3& halfExtents,
    const glm::vec3& color)
{
    // Add draw call data
    DebugMeshDrawCallData drawCallData{};

    drawCallData.meshID = this->boxMeshID;
    drawCallData.pushConstantData.transform = 
        glm::translate(glm::mat4(1.0f), position) *
        glm::scale(glm::mat4(1.0f), halfExtents);
    drawCallData.pushConstantData.color = glm::vec4(color, 1.0f);

    this->meshDrawData.push_back(drawCallData);
}

void DebugRenderer::renderSkeleton(
    const uint32_t& meshID,
    const Transform& transformComp,
    const AnimationComponent& animComp,
    const glm::vec3& color)
{
    // Mesh info
    Mesh& mesh = this->resourceManager->getMesh(meshID);
    MeshData& meshData = mesh.getMeshData();

    // Loop through bones and render them as lines
    size_t numBones = meshData.bones.size();
    glm::vec3 bonePos(0.0f);
    glm::vec3 boneDir(0.0f, 0.0f, 1.0f);
    glm::mat4 boneLocalTransform(1.0f);
    for (size_t i = 0; i < numBones; ++i)
    {
        // Bone local transform
        boneLocalTransform = meshData.bones[i].boneMatrix;

        // Bone position
        bonePos = SMath::extractTranslation(boneLocalTransform);

        // Bone direction
        boneDir = boneLocalTransform * glm::vec4(0.0f, 1.0f / transformComp.scale.y, 0.0f, 0.0f);

        // Transform by model matrix
        bonePos = transformComp.matrix * glm::vec4(bonePos, 1.0f);
        boneDir = transformComp.matrix * glm::vec4(boneDir, 0.0f);

        // Render line
        this->renderLine(bonePos, bonePos + boneDir, color);
    }
}