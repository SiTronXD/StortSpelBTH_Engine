#include "DebugRenderer.hpp"
#include "vulkan/UniformBufferStructs.hpp"
#include "../dev/Log.hpp"

DebugRenderer::DebugRenderer()
    : physicalDevice(nullptr),
    device(nullptr),
    vma(nullptr),
    resourceManager(nullptr),
    renderPass(nullptr),
    framesInFlight(0)
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

    // Vertex buffers
    this->vertexBuffers.create(
        *this->device,
        *this->vma,
        *this->transferQueue,
        *this->transferCommandPool
    );
    this->vertexStreams.positions.reserve(START_NUM_MAX_ELEMENTS);
    this->vertexStreams.positions.push_back(glm::vec3(-10.0f, -10.0f, 35.0f));
    this->vertexStreams.positions.push_back(glm::vec3(10.0f, 10.0f, 25.0f));
    this->vertexBuffers.addVertexBuffer(this->vertexStreams.positions);

    // Shader input
    this->shaderInput.beginForInput(
        *this->physicalDevice,
        *this->device,
        *this->vma,
        *this->resourceManager,
        this->framesInFlight);
    this->viewProjectionUB =
        this->shaderInput.addUniformBuffer(sizeof(UboViewProjection));
    this->shaderInput.endForInput();

    // Pipeline
    this->pipeline.createPipeline(
        *this->device,
        this->shaderInput,
        *this->renderPass,
        vertexStreams,
        "dbgLine.vert.spv",
        "dbgLine.frag.spv",
        false,
        vk::PrimitiveTopology::eLineList
    );
}

void DebugRenderer::cleanup()
{
    this->vertexBuffers.cleanup();
    this->pipeline.cleanup();
    this->shaderInput.cleanup();
}

void DebugRenderer::beginDebugRender()
{
    // Clear CPU buffer contents
    this->vertexStreams.positions.clear();
}

void DebugRenderer::renderLine(
    const glm::vec3& pos0, 
    const glm::vec3& pos1)
{
    if (this->vertexStreams.positions.size() >= START_NUM_MAX_ELEMENTS)
    {
        Log::error("Reached maximum number of rendered ui elements.");
        return;
    }

    // Add positions
    this->vertexStreams.positions.push_back(pos0);
    this->vertexStreams.positions.push_back(pos1);
}

void DebugRenderer::endDebugRender()
{

}