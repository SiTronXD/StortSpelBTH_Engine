#include "UIRenderer.hpp"
#include "../resource_management/ResourceManager.hpp"

void UIRenderer::createVertexBuffers(
    Device& device,
    VmaAllocator& vma,
    vk::Queue& transferQueue,
    vk::CommandPool& transferCommandPool,
    const uint32_t& framesInFlight)
{
    this->vertexBuffers.resize(framesInFlight);
    this->vertexBufferMemories.resize(framesInFlight);

    // Temporary positions
    std::vector<glm::vec3> dataStream =
    {
        glm::vec3(-0.5f, 0.5f, 0.5f),
        glm::vec3( 0.5f, 0.5f, 0.5f),
        glm::vec3(-0.5f, -0.5f, 0.5f)
    };

    // Create one vertex buffer for each frame in flight
    for (uint32_t i = 0; i < framesInFlight; ++i)
    {
        // Temporary buffer to "Stage" vertex data before transferring to GPU
        vk::Buffer stagingBuffer;
        VmaAllocation stagingBufferMemory{};
        VmaAllocationInfo allocInfo_staging;

        vk::DeviceSize bufferSize = sizeof(dataStream[0]) * dataStream.size();

        Buffer::createBuffer(
            {
                .bufferSize = bufferSize,
                .bufferUsageFlags = vk::BufferUsageFlagBits::eTransferSrc,       // This buffers vertex data will be transfered somewhere else!
                .bufferProperties = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                                    | VMA_ALLOCATION_CREATE_MAPPED_BIT,
                .buffer = &stagingBuffer,
                .bufferMemory = &stagingBufferMemory,
                .allocationInfo = &allocInfo_staging,
                .vma = &vma
            });

        // -- Map memory to our Temporary Staging Vertex Buffer -- 
        void* data{};
        if (vmaMapMemory(vma, stagingBufferMemory, &data) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate Mesh Staging Vertex Buffer Using VMA!");
        };

        memcpy(
            data,
            dataStream.data(),
            (size_t)bufferSize
        );

        vmaUnmapMemory(vma, stagingBufferMemory);

        VmaAllocationInfo allocInfo_deviceOnly;
        // Create Buffer with TRANSFER_DST_BIT to mark as recipient of transfer data (also VERTEX_BUFFER)
        // Buffer memory is to be DEVICVE_LOCAL_BIT meaning memory is on the GPU and only accesible by it and not the CPU (HOST)
        Buffer::createBuffer(
            {
                .bufferSize = bufferSize,
                .bufferUsageFlags = vk::BufferUsageFlagBits::eTransferDst        // Destination Buffer to be transfered to
                                    | vk::BufferUsageFlagBits::eVertexBuffer,    // This is a Vertex Buffer
                .bufferProperties = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
                .buffer = &this->vertexBuffers[i],
                .bufferMemory = &this->vertexBufferMemories[i],
                .allocationInfo = &allocInfo_deviceOnly,
                .vma = &vma
            });

        // Copy Staging Buffer to Vertex Buffer on GPU
        Buffer::copyBuffer(
            device.getVkDevice(),
            transferQueue,
            transferCommandPool,
            stagingBuffer,
            this->vertexBuffers[i],
            bufferSize
        );

        // Clean up Staging Buffer stuff
        device.getVkDevice().destroyBuffer(stagingBuffer);
        vmaFreeMemory(vma, stagingBufferMemory);
    }
}

UIRenderer::UIRenderer()
    : device(nullptr),
    vma(nullptr)
{
}

void UIRenderer::create(
	PhysicalDevice& physicalDevice,
	Device& device,
	VmaAllocator& vma,
	ResourceManager& resourceManager,
	vk::RenderPass& renderPass,
    vk::Queue& transferQueue,
    vk::CommandPool& transferCommandPool,
	const uint32_t& framesInFlight)
{
    this->device = &device;
    this->vma = &vma;

	// Target data from the vertex buffers
	VertexStreams targetVertexStream{};
	targetVertexStream.positions.resize(1);

	// Shader input, with no inputs for now
	this->uiShaderInput.beginForInput(
		physicalDevice,
		device,
		vma,
		resourceManager,
		framesInFlight);
	this->uiShaderInput.endForInput();

	// Pipeline
	this->uiPipeline.createPipeline(
		device,
		this->uiShaderInput,
		renderPass,
		targetVertexStream,
		"ui.vert.spv",
		"ui.frag.spv"
	);

    // Create vertex buffer for each frame in flight
	this->createVertexBuffers(
        device,
        vma,
        transferQueue,
        transferCommandPool,
        framesInFlight
    );
}

void UIRenderer::cleanup()
{
    for (size_t i = 0; i < this->vertexBuffers.size(); ++i)
    {
        this->device->getVkDevice().destroyBuffer(this->vertexBuffers[i]);
        vmaFreeMemory(*this->vma, this->vertexBufferMemories[i]);
    }

	this->uiPipeline.cleanup();
	this->uiShaderInput.cleanup();
}