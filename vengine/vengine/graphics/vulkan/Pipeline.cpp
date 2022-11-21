#include "pch.h"
#include "Pipeline.hpp"

#include "RenderPass.hpp"
#include "../MeshData.hpp"
#include "../../dev/Log.hpp"

vk::ShaderModule Pipeline::createShaderModule(
    const std::vector<char>& code)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    vk::ShaderModuleCreateInfo shaderCreateInfo{};
    shaderCreateInfo.setCodeSize(code.size());
    shaderCreateInfo.setPCode(
        reinterpret_cast<const uint32_t*>(code.data())
    );    // pointer of code (of uint32_t pointe rtype //NOLINT:Ok to use Reinterpret cast here

    vk::ShaderModule shaderModule = 
        this->device->getVkDevice().createShaderModule(
            shaderCreateInfo
        );
    return shaderModule;
}

template <typename T>
void Pipeline::insertBindingFromStream(
    const std::vector<T>& dataStream,
    std::vector<vk::VertexInputBindingDescription>&
        outputBindingDescs)
{
    // Empty data stream
    if (dataStream.size() <= 0)
    {
        return;
    }

    uint32_t bindingIndex = outputBindingDescs.size();

    outputBindingDescs.push_back(vk::VertexInputBindingDescription());
    outputBindingDescs[bindingIndex].setBinding(uint32_t(bindingIndex));                 // Can bind multiple streams of data, this defines which one.
    outputBindingDescs[bindingIndex].setStride(sizeof(dataStream[0]));    // Size of a single Vertex Object
    outputBindingDescs[bindingIndex].setInputRate(vk::VertexInputRate::eVertex); // How to move between data after each vertex...
}

template <typename T>
void Pipeline::insertAttributeFromStream(
    const std::vector<T>& dataStream,
    const vk::Format& format,
    std::vector<vk::VertexInputAttributeDescription>&
        outputAttributeDescs)
{
    // Empty data stream
    if (dataStream.size() <= 0)
    {
        return;
    }

    uint32_t attributeIndex = outputAttributeDescs.size();

    outputAttributeDescs.push_back(vk::VertexInputAttributeDescription());
    outputAttributeDescs[attributeIndex].setBinding(uint32_t(attributeIndex));
    outputAttributeDescs[attributeIndex].setLocation(uint32_t(attributeIndex));
    outputAttributeDescs[attributeIndex].setFormat(format);
    outputAttributeDescs[attributeIndex].setOffset(0);
}

Pipeline::Pipeline()
	: device(nullptr),
    hasBeenCreated(false)
{
}

Pipeline::~Pipeline()
{
}

void Pipeline::createPipeline(
    Device& device,
    ShaderInput& shaderInput,
    RenderPass& renderPass,
    const VertexStreams& targetVertexStream,
    const std::string& vertexShaderName,
    const std::string& fragmentShaderName,
    const bool& depthTestingEnabled,
    const bool& wireframe,
    const bool& backfaceCulling,
    const vk::PrimitiveTopology& topology)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

	this->device = &device;

    bool hasFragmentShader = fragmentShaderName != "";

    // read in SPIR-V code of shaders
    auto vertexShaderCode = vengine_helper::readShaderFile(vertexShaderName);
    std::vector<char> fragShaderCode;
    if (hasFragmentShader)
    {
        fragShaderCode = vengine_helper::readShaderFile(fragmentShaderName);
    }

    // Build Shader Modules to link to Graphics Pipeline
    // Create Shader Modules
    vk::ShaderModule vertexShaderModule = this->createShaderModule(vertexShaderCode);
    VulkanDbg::registerVkObjectDbgInfo("ShaderModule VertexShader", vk::ObjectType::eShaderModule, reinterpret_cast<uint64_t>(vk::ShaderModule::CType(vertexShaderModule)));
    
    // --- SHADER STAGE CREATION INFORMATION ---
    // Vertex Stage Creation Information
    vk::PipelineShaderStageCreateInfo vertexShaderStageCreateInfo{};
    vertexShaderStageCreateInfo.setStage(vk::ShaderStageFlagBits::eVertex);                             // Shader stage name
    vertexShaderStageCreateInfo.setModule(vertexShaderModule);                                    // Shader Modual to be used by Stage
    vertexShaderStageCreateInfo.setPName("main");                                                 //Name of the vertex Shaders main function (function to run)

    // Put shader stage creation infos into array
    // graphics pipeline creation info requires array of shader stage creates
    std::vector<vk::PipelineShaderStageCreateInfo> pipelineShaderStageCreateInfos
    {
        vertexShaderStageCreateInfo,
    };

    vk::ShaderModule fragmentShaderModule{};
    if (hasFragmentShader)
    {
        fragmentShaderModule = this->createShaderModule(fragShaderCode);
        VulkanDbg::registerVkObjectDbgInfo("ShaderModule fragmentShader", vk::ObjectType::eShaderModule, reinterpret_cast<uint64_t>(vk::ShaderModule::CType(fragmentShaderModule)));

        // Fragment Stage Creation Information
        vk::PipelineShaderStageCreateInfo fragmentShaderPipelineCreatInfo{};
        fragmentShaderPipelineCreatInfo.setStage(vk::ShaderStageFlagBits::eFragment);                       // Shader Stage Name
        fragmentShaderPipelineCreatInfo.setModule(fragmentShaderModule);                              // Shader Module used by stage
        fragmentShaderPipelineCreatInfo.setPName("main");                                             // name of the fragment shader main function (function to run)

        // Add pipeline shader stage create info
        pipelineShaderStageCreateInfos.push_back(
            fragmentShaderPipelineCreatInfo
        );
    }

    // -- FIXED SHADER STAGE CONFIGURATIONS -- 

    // How the data for a single vertex (including info such as position, colour, texture coords, normals, etc)
    // is as a whole.
    std::vector<vk::VertexInputBindingDescription> bindingDescriptions;
    this->insertBindingFromStream(targetVertexStream.positions, bindingDescriptions);
    this->insertBindingFromStream(targetVertexStream.normals, bindingDescriptions);
    this->insertBindingFromStream(targetVertexStream.colors, bindingDescriptions);
    this->insertBindingFromStream(targetVertexStream.texCoords, bindingDescriptions);
    this->insertBindingFromStream(targetVertexStream.boneWeights, bindingDescriptions);
    this->insertBindingFromStream(targetVertexStream.boneIndices, bindingDescriptions);

    // How the data for an attribute is definied within a vertex    
    std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
    this->insertAttributeFromStream(targetVertexStream.positions, vk::Format::eR32G32B32Sfloat, attributeDescriptions);
    this->insertAttributeFromStream(targetVertexStream.normals, vk::Format::eR32G32B32Sfloat, attributeDescriptions);
    this->insertAttributeFromStream(targetVertexStream.colors, vk::Format::eR32G32B32Sfloat, attributeDescriptions);
    this->insertAttributeFromStream(targetVertexStream.texCoords, vk::Format::eR32G32Sfloat, attributeDescriptions);
    this->insertAttributeFromStream(targetVertexStream.boneWeights, vk::Format::eR32G32B32A32Sfloat, attributeDescriptions);
    this->insertAttributeFromStream(targetVertexStream.boneIndices, vk::Format::eR32G32B32A32Uint, attributeDescriptions);


    // -- VERTEX INPUT --
    vk::PipelineVertexInputStateCreateInfo vertexInputCreateInfo;
    vertexInputCreateInfo.setVertexBindingDescriptionCount(static_cast<uint32_t>(bindingDescriptions.size()));
    vertexInputCreateInfo.setPVertexBindingDescriptions(bindingDescriptions.data());
    vertexInputCreateInfo.setVertexAttributeDescriptionCount(static_cast<uint32_t>(attributeDescriptions.size()));
    vertexInputCreateInfo.setPVertexAttributeDescriptions(attributeDescriptions.data());

    // -- INPUT ASSEMBLY --
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo;
    inputAssemblyStateCreateInfo.setTopology(topology);        // Primitive type to assemble primitives from ;
    inputAssemblyStateCreateInfo.setPrimitiveRestartEnable(VK_FALSE);                     // Allow overrideing tof "strip" topology to start new primitives

    // -- VIEWPORT & SCISSOR ---
    // Only count is necessary, since viewport and scissor are dynamic
    vk::PipelineViewportStateCreateInfo viewportStateCreateInfo = {};
    viewportStateCreateInfo.setViewportCount(uint32_t(1));
    viewportStateCreateInfo.setScissorCount(uint32_t(1));

    // --- DYNAMIC STATES ---
    // dynamic states to enable... NOTE: to change some stuff you might need to reset it first... example viewport...
    std::vector<vk::DynamicState > dynamicStateEnables;
    dynamicStateEnables.push_back(vk::DynamicState::eViewport); // Dynamic Viewport    : can resize in command buffer with vkCmdSetViewport(command, 0,1,&viewport)
    dynamicStateEnables.push_back(vk::DynamicState::eScissor);  // Dynamic Scissor     : Can resize in commandbuffer with vkCmdSetScissor(command, 0,1, &viewport)

    // Dynamic state creation info
    vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
    dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
    dynamicStateCreateInfo.pDynamicStates = dynamicStateEnables.data();

    // --- RASTERIZER ---
    vk::PipelineRasterizationStateCreateInfo rasterizationStateCreateInfo;
    rasterizationStateCreateInfo.setDepthClampEnable(VK_FALSE);         // Change if fragments beyond near/far planes are clipped / clamped  (Requires depthclamp as a device features...)
    rasterizationStateCreateInfo.setRasterizerDiscardEnable(VK_FALSE);// Wether to discard data or skip rasterizzer. Never creates fragments, only suitable for pipleine without framebuffer output
    rasterizationStateCreateInfo.setPolygonMode(wireframe ? vk::PolygonMode::eLine : vk::PolygonMode::eFill);// How to handle filling point betweeen vertices...
    rasterizationStateCreateInfo.setLineWidth(1.F);                  // how thick lines should be when drawn (other than 1 requires device features...)
    rasterizationStateCreateInfo.setCullMode(backfaceCulling ? vk::CullModeFlagBits::eBack : vk::CullModeFlagBits::eNone);  // Which face of tri to cull
    rasterizationStateCreateInfo.setFrontFace(vk::FrontFace::eCounterClockwise); // Winding order to determine which side is front
    rasterizationStateCreateInfo.setDepthBiasEnable(VK_FALSE);        // Wether to add depthbiaoas to fragments (to remove shadowacne...)

    // --- MULTISAMPLING ---
    vk::PipelineMultisampleStateCreateInfo multisampleStateCreateInfo;
    multisampleStateCreateInfo.setSampleShadingEnable(VK_FALSE);                      // Enable multisample shading or not
    multisampleStateCreateInfo.setRasterizationSamples(vk::SampleCountFlagBits::e1);        // number of samples to use per fragment


    // --- BLENDING --

    // Blend attachment State (how blending is handled)
    vk::PipelineColorBlendAttachmentState  colorState;
    colorState.setColorWriteMask(vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eB // Colours to apply blending to
        | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eR);
    colorState.setBlendEnable(VK_TRUE);                                                       // enable blending

    // Blending uses Equation: (srcColorBlendFactor * new Colour) colorBlendOp (dstColorBlendFactor * old Colour)
    colorState.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha);
    colorState.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha);
    colorState.setColorBlendOp(vk::BlendOp::eAdd);

    colorState.setSrcAlphaBlendFactor(vk::BlendFactor::eOne);
    colorState.setDstAlphaBlendFactor(vk::BlendFactor::eZero);
    colorState.setAlphaBlendOp(vk::BlendOp::eAdd);

    vk::PipelineColorBlendStateCreateInfo colorBlendingCreateInfo;
    colorBlendingCreateInfo.setLogicOpEnable(VK_FALSE);               // Alternative to calculations is to use logical Operations
    colorBlendingCreateInfo.setAttachmentCount(uint32_t(1));
    colorBlendingCreateInfo.setPAttachments(&colorState);

    // --- DEPTH STENCIL TESING ---
    vk::PipelineDepthStencilStateCreateInfo depthStencilCreateInfo{};
    depthStencilCreateInfo.setDepthTestEnable(depthTestingEnabled ? VK_TRUE : VK_FALSE);              // Enable Depth Testing; Check the Depth to determine if it should write to the fragment
    depthStencilCreateInfo.setDepthWriteEnable(VK_TRUE);              // enable writing to Depth Buffer; To make sure it replaces old values
    depthStencilCreateInfo.setDepthCompareOp(vk::CompareOp::eLess);   // Describes that we want to replace the old values IF new values are smaller/lesser.
    depthStencilCreateInfo.setDepthBoundsTestEnable(VK_FALSE);             // In case we want to use as Min and a Max Depth; if depth Values exist between two bounds... 
    depthStencilCreateInfo.setStencilTestEnable(VK_FALSE);             // Whether to enable the Stencil Test; we dont use it so we let it be disabled
    
    // -- GRAPHICS PIPELINE CREATION --
    vk::GraphicsPipelineCreateInfo pipelineCreateInfo;
    pipelineCreateInfo.setStageCount(static_cast<uint32_t>(pipelineShaderStageCreateInfos.size()));                                          // Number of shader stages
    pipelineCreateInfo.setPStages(pipelineShaderStageCreateInfos.data());         // List of Shader stages
    pipelineCreateInfo.setPVertexInputState(&vertexInputCreateInfo);              // All the fixed function Pipeline states
    pipelineCreateInfo.setPInputAssemblyState(&inputAssemblyStateCreateInfo);
    pipelineCreateInfo.setPViewportState(&viewportStateCreateInfo);
    pipelineCreateInfo.setPDynamicState(&dynamicStateCreateInfo);
    pipelineCreateInfo.setPRasterizationState(&rasterizationStateCreateInfo);
    pipelineCreateInfo.setPMultisampleState(&multisampleStateCreateInfo);
    pipelineCreateInfo.setPColorBlendState(&colorBlendingCreateInfo);
    pipelineCreateInfo.setPDepthStencilState(&depthStencilCreateInfo);
    pipelineCreateInfo.setLayout(shaderInput.getPipelineLayout().getVkPipelineLayout());                                 // Pipeline layout pipeline should use
    pipelineCreateInfo.setRenderPass(renderPass.getVkRenderPass());                                 // Render pass description the pipeline is compatible with
    pipelineCreateInfo.setSubpass(uint32_t(0));                                             // subpass of render pass to use with pipeline

    // Pipeline Derivatives : Can Create multiple pipelines that derive from one another for optimization
    pipelineCreateInfo.setBasePipelineHandle(nullptr); // Existing pipeline to derive from ...
    pipelineCreateInfo.setBasePipelineIndex(int32_t(-1));              // or index of pipeline being created to derive from ( in case of creating multiple of at once )

    // Create Graphics Pipeline
    vk::Result result{};
    std::tie(result, this->pipeline) = 
        this->device->getVkDevice().createGraphicsPipeline(
            nullptr, 
            pipelineCreateInfo
        );
    if (result != vk::Result::eSuccess)
    {
        Log::error("Could not create pipeline");
    }
    VulkanDbg::registerVkObjectDbgInfo("VkPipeline", vk::ObjectType::ePipeline, reinterpret_cast<uint64_t>(vk::Pipeline::CType(this->pipeline)));

    //Destroy Shader Moduels, no longer needed after pipeline created
    this->device->getVkDevice().destroyShaderModule(vertexShaderModule);
    this->device->getVkDevice().destroyShaderModule(fragmentShaderModule);

    this->hasBeenCreated = true;
}

void Pipeline::cleanup()
{
    // Only cleanup if a pipeline has been created
    if (!this->hasBeenCreated) return;

    this->hasBeenCreated = false;

	this->device->getVkDevice().destroyPipeline(this->pipeline);
}