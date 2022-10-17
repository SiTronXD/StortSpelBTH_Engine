#include "DebugRenderer.hpp"

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
    const uint32_t& framesInFlight)
{
    this->physicalDevice = &physicalDevice;
    this->device = &device;
    this->vma = &vma;
    this->resourceManager = &resourceManager;
    this->renderPass = &renderPass;
    this->framesInFlight = framesInFlight;
}

void DebugRenderer::initForScene()
{
    // Cleanup shader input if possible
    this->cleanup();

    // Shader input
    this->shaderInput.beginForInput(
        *this->physicalDevice,
        *this->device,
        *this->vma,
        *this->resourceManager,
        this->framesInFlight);

    this->shaderInput.endForInput();

    // Pipeline
    this->pipeline.createPipeline(
        *this->device,
        this->shaderInput,
        *this->renderPass,
        VertexStreams{},
        "dbgLine.vert.spv",
        "dbgLine.frag.spv",
        false
    );
}

void DebugRenderer::cleanup()
{
    this->pipeline.cleanup();
    this->shaderInput.cleanup();
}