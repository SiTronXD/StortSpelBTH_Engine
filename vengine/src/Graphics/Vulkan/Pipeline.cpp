#include "Pipeline.hpp"

#include "Device.hpp"
#include "VulkanDbg.hpp"
#include "../../Dev/Log.hpp"

Pipeline::Pipeline()
	: device(nullptr)
{
}

Pipeline::~Pipeline()
{
}

void Pipeline::createPipeline(Device& device)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

	this->device = &device;

    // read in SPIR-V code of shaders
    auto vertexShaderCode = vengine_helper::readShaderFile("shader.vert.spv");
    auto fragShaderCode = vengine_helper::readShaderFile("shader.frag.spv");

    // Build Shader Modules to link to Graphics Pipeline
    // Create Shader Modules
    vk::ShaderModule vertexShaderModule = this->createShaderModule(vertexShaderCode);
    vk::ShaderModule fragmentShaderModule = this->createShaderModule(fragShaderCode);
    VulkanDbg::registerVkObjectDbgInfo("ShaderModule VertexShader", vk::ObjectType::eShaderModule, reinterpret_cast<uint64_t>(vk::ShaderModule::CType(vertexShaderModule)));
    VulkanDbg::registerVkObjectDbgInfo("ShaderModule fragmentShader", vk::ObjectType::eShaderModule, reinterpret_cast<uint64_t>(vk::ShaderModule::CType(fragmentShaderModule)));

    // --- SHADER STAGE CREATION INFORMATION ---
    // Vertex Stage Creation Information
    vk::PipelineShaderStageCreateInfo vertexShaderStageCreateInfo{};
    vertexShaderStageCreateInfo.setStage(vk::ShaderStageFlagBits::eVertex);                             // Shader stage name
    vertexShaderStageCreateInfo.setModule(vertexShaderModule);                                    // Shader Modual to be used by Stage
    vertexShaderStageCreateInfo.setPName("main");                                                 //Name of the vertex Shaders main function (function to run)

    // Fragment Stage Creation Information
    vk::PipelineShaderStageCreateInfo fragmentShaderPipelineCreatInfo{};
    fragmentShaderPipelineCreatInfo.setStage(vk::ShaderStageFlagBits::eFragment);                       // Shader Stage Name
    fragmentShaderPipelineCreatInfo.setModule(fragmentShaderModule);                              // Shader Module used by stage
    fragmentShaderPipelineCreatInfo.setPName("main");                                             // name of the fragment shader main function (function to run)

    // Put shader stage creation infos into array
    // graphics pipeline creation info requires array of shader stage creates
    std::array<vk::PipelineShaderStageCreateInfo, 2> pipelineShaderStageCreateInfos
    {
        vertexShaderStageCreateInfo,
        fragmentShaderPipelineCreatInfo
    };

    // -- FIXED SHADER STAGE CONFIGURATIONS -- 

    // How the data for a single vertex (including info such as position, colour, texture coords, normals, etc)
    // is as a whole.
    vk::VertexInputBindingDescription bindingDescription;
    bindingDescription.setBinding(uint32_t(0));                 // Can bind multiple streams of data, this defines which one.
    bindingDescription.setStride(sizeof(Vertex));    // Size of a single Vertex Object
    bindingDescription.setInputRate(vk::VertexInputRate::eVertex); // How to move between data after each vertex...

    // How the Data  for an attribute is definied within a vertex    
    std::array<vk::VertexInputAttributeDescription, 3> attributeDescriptions{};

    // Position Attribute: 
    attributeDescriptions[0].setBinding(uint32_t(0));                           // which binding the data is at (should be same as above)
    attributeDescriptions[0].setLocation(uint32_t(0));                           // Which Location in shader where data will be read from
    attributeDescriptions[0].setFormat(vk::Format::eR32G32B32Sfloat);  // Format the data will take (also helps define size of data)
    attributeDescriptions[0].setOffset(offsetof(Vertex, pos));       // Sets the offset of our struct member Pos (where this attribute is defined for a single vertex...)

    // Color Attribute.
    attributeDescriptions[1].setBinding(uint32_t(0));
    attributeDescriptions[1].setLocation(uint32_t(1));
    attributeDescriptions[1].setFormat(vk::Format::eR32G32B32Sfloat);
    attributeDescriptions[1].setOffset(offsetof(Vertex, col));

    // Texture Coorinate Attribute (uv): 
    attributeDescriptions[2].setBinding(uint32_t(0));
    attributeDescriptions[2].setLocation(uint32_t(2));
    attributeDescriptions[2].setFormat(vk::Format::eR32G32Sfloat);      // Note; only RG, since it's a 2D image we don't use the depth and thus we only need RG and not RGB
    attributeDescriptions[2].setOffset(offsetof(Vertex, tex));


    // -- VERTEX INPUT --
    vk::PipelineVertexInputStateCreateInfo vertexInputCreateInfo;
    vertexInputCreateInfo.setVertexBindingDescriptionCount(uint32_t(1));
    vertexInputCreateInfo.setPVertexBindingDescriptions(&bindingDescription);
    vertexInputCreateInfo.setVertexAttributeDescriptionCount(static_cast<uint32_t>(attributeDescriptions.size()));
    vertexInputCreateInfo.setPVertexAttributeDescriptions(attributeDescriptions.data());

    // -- INPUT ASSEMBLY --
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo;
    inputAssemblyStateCreateInfo.setTopology(vk::PrimitiveTopology::eTriangleList);        // Primitive type to assemble primitives from ;
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
    rasterizationStateCreateInfo.setPolygonMode(vk::PolygonMode::eFill);// How to handle filling point betweeen vertices...
    rasterizationStateCreateInfo.setLineWidth(1.F);                  // how thiock lines should be when drawn (other than 1 requires device features...)
    rasterizationStateCreateInfo.setCullMode(vk::CullModeFlagBits::eBack);  // Which face of tri to cull
    rasterizationStateCreateInfo.setFrontFace(vk::FrontFace::eCounterClockwise);// Since our Projection matrix now has a inverted Y-axis (GLM is right handed, but vulkan is left handed)
    // winding order to determine which side is front
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

    // --- PIPELINE LAYOUT ---

    // We have two Descriptor Set Layouts, 
    // One for View and Projection matrices Uniform Buffer, and the other one for the texture sampler!
    std::array<vk::DescriptorSetLayout, 2> descriptorSetLayouts
    {
        this->descriptorSetLayout,
        this->samplerDescriptorSetLayout
    };

    vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.setSetLayoutCount(static_cast<uint32_t>(descriptorSetLayouts.size()));
    pipelineLayoutCreateInfo.setPSetLayouts(descriptorSetLayouts.data());
    pipelineLayoutCreateInfo.setPushConstantRangeCount(uint32_t(1));                    // We only have One Push Constant range we want to use
    pipelineLayoutCreateInfo.setPPushConstantRanges(&this->pushConstantRange);// the Push Constant Range we want to use

    this->pipelineLayout = this->device->getVkDevice().createPipelineLayout(pipelineLayoutCreateInfo);
    VulkanDbg::registerVkObjectDbgInfo("VkPipelineLayout GraphicsPipelineLayout", vk::ObjectType::ePipelineLayout, reinterpret_cast<uint64_t>(vk::PipelineLayout::CType(this->pipelineLayout)));
    
    // --- DEPTH STENCIL TESING ---
    vk::PipelineDepthStencilStateCreateInfo depthStencilCreateInfo{};
    depthStencilCreateInfo.setDepthTestEnable(VK_TRUE);              // Enable Depth Testing; Check the Depth to determine if it should write to the fragment
    depthStencilCreateInfo.setDepthWriteEnable(VK_TRUE);              // enable writing to Depth Buffer; To make sure it replaces old values
    depthStencilCreateInfo.setDepthCompareOp(vk::CompareOp::eLess);   // Describes that we want to replace the old values IF new values are smaller/lesser.
    depthStencilCreateInfo.setDepthBoundsTestEnable(VK_FALSE);             // In case we want to use as Min and a Max Depth; if depth Values exist between two bounds... 
    depthStencilCreateInfo.setStencilTestEnable(VK_FALSE);             // Whether to enable the Stencil Test; we dont use it so we let it be disabled
    
    // -- GRAPHICS PIPELINE CREATION --
    vk::GraphicsPipelineCreateInfo pipelineCreateInfo;
    pipelineCreateInfo.setStageCount(uint32_t(2));                                          // Number of shader stages
    pipelineCreateInfo.setPStages(pipelineShaderStageCreateInfos.data());         // List of Shader stages
    pipelineCreateInfo.setPVertexInputState(&vertexInputCreateInfo);              // All the fixed function Pipeline states
    pipelineCreateInfo.setPInputAssemblyState(&inputAssemblyStateCreateInfo);
    pipelineCreateInfo.setPViewportState(&viewportStateCreateInfo);
    pipelineCreateInfo.setPDynamicState(&dynamicStateCreateInfo);
    pipelineCreateInfo.setPRasterizationState(&rasterizationStateCreateInfo);
    pipelineCreateInfo.setPMultisampleState(&multisampleStateCreateInfo);
    pipelineCreateInfo.setPColorBlendState(&colorBlendingCreateInfo);
    pipelineCreateInfo.setPDepthStencilState(&depthStencilCreateInfo);
    pipelineCreateInfo.setLayout(this->pipelineLayout);                                 // Pipeline layout pipeline should use
    pipelineCreateInfo.setRenderPass(this->renderPass_base);                                 // Render pass description the pipeline is compatible with
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
    VulkanDbg::registerVkObjectDbgInfo("VkPipeline GraphicsPipeline", vk::ObjectType::ePipeline, reinterpret_cast<uint64_t>(vk::Pipeline::CType(this->graphicsPipeline)));

    //Destroy Shader Moduels, no longer needed after pipeline created
    this->device->getVkDevice().destroyShaderModule(vertexShaderModule);
    this->device->getVkDevice().destroyShaderModule(fragmentShaderModule);
}

void Pipeline::cleanup()
{
	this->device->getVkDevice().destroyPipeline(this->pipeline);
}