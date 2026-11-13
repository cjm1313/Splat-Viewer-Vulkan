#include "vk/myPipeline.h"
#include "vk/vulkanContext.h"

#include <fstream>
#include <stdexcept>

PipelineConfigInfo PipelineConfigInfo::defaultConfig(vk::Format colorFormat) {
    PipelineConfigInfo config{};

    config.vertShaderPath = std::string(SHADER_DIR) + "shader.vert.spv";
    config.fragShaderPath = std::string(SHADER_DIR) + "shader.frag.spv";

    config.inputAssembly = vk::PipelineInputAssemblyStateCreateInfo{
        .topology = vk::PrimitiveTopology::eTriangleList
    };

    config.viewportState = vk::PipelineViewportStateCreateInfo{
        .viewportCount = 1,
        .scissorCount  = 1
    };

    config.rasterizer = vk::PipelineRasterizationStateCreateInfo{
        .depthClampEnable        = vk::False,
        .rasterizerDiscardEnable = vk::False,
        .polygonMode             = vk::PolygonMode::eFill,
        .cullMode                = vk::CullModeFlagBits::eBack,
        .frontFace               = vk::FrontFace::eClockwise,
        .depthBiasEnable         = vk::False,
        .lineWidth               = 1.0f
    };

    config.multisampling = vk::PipelineMultisampleStateCreateInfo{
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .sampleShadingEnable  = vk::False
    };

    config.colorBlendAttachment = vk::PipelineColorBlendAttachmentState{
        .blendEnable    = vk::False,
        .colorWriteMask = vk::ColorComponentFlagBits::eR |
                          vk::ColorComponentFlagBits::eG |
                          vk::ColorComponentFlagBits::eB |
                          vk::ColorComponentFlagBits::eA
    };

    config.dynamicStates = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    config.colorAttachmentFormat = colorFormat;

    return config;
}

MyPipeline::MyPipeline(const VulkanContext& ctx, const PipelineConfigInfo& config) {
    create(ctx.getDevice(), config);
}

void MyPipeline::create(const vk::raii::Device& device, const PipelineConfigInfo& config) {
    vk::raii::ShaderModule shaderModuleVert = createShaderModule(device, readFile(config.vertShaderPath));
    vk::raii::ShaderModule shaderModuleFrag = createShaderModule(device, readFile(config.fragShaderPath));

    vk::PipelineShaderStageCreateInfo vertShaderStageInfo{
        .stage  = vk::ShaderStageFlagBits::eVertex,
        .module = shaderModuleVert,
        .pName  = "main"
    };
    vk::PipelineShaderStageCreateInfo fragShaderStageInfo{
        .stage  = vk::ShaderStageFlagBits::eFragment,
        .module = shaderModuleFrag,
        .pName  = "main"
    };
    vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
        .vertexBindingDescriptionCount   = static_cast<uint32_t>(config.bindingDescriptions.size()),
        .pVertexBindingDescriptions      = config.bindingDescriptions.data(),
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(config.attributeDescriptions.size()),
        .pVertexAttributeDescriptions    = config.attributeDescriptions.data()
    };

    vk::PipelineColorBlendStateCreateInfo colorBlending{
        .logicOpEnable   = vk::False,
        .logicOp         = vk::LogicOp::eCopy,
        .attachmentCount = 1,
        .pAttachments    = &config.colorBlendAttachment
    };

    vk::PipelineDynamicStateCreateInfo dynamicState{
        .dynamicStateCount = static_cast<uint32_t>(config.dynamicStates.size()),
        .pDynamicStates    = config.dynamicStates.data()
    };

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
        .setLayoutCount         = static_cast<uint32_t>(config.descriptorSetLayouts.size()),
        .pSetLayouts            = config.descriptorSetLayouts.data(),
        .pushConstantRangeCount = static_cast<uint32_t>(config.pushConstantRanges.size()),
        .pPushConstantRanges    = config.pushConstantRanges.data()
    };
    pipelineLayout = vk::raii::PipelineLayout(device, pipelineLayoutInfo);

    vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain = {
        {
            .stageCount          = 2,
            .pStages             = shaderStages,
            .pVertexInputState   = &vertexInputInfo,
            .pInputAssemblyState = &config.inputAssembly,
            .pViewportState      = &config.viewportState,
            .pRasterizationState = &config.rasterizer,
            .pMultisampleState   = &config.multisampling,
            .pColorBlendState    = &colorBlending,
            .pDynamicState       = &dynamicState,
            .layout              = pipelineLayout,
            .renderPass          = nullptr
        },
        {
            .colorAttachmentCount    = 1,
            .pColorAttachmentFormats = &config.colorAttachmentFormat
        }
    };

    graphicsPipeline = vk::raii::Pipeline(device, nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());
}

const vk::raii::Pipeline& MyPipeline::getPipeline() const {
    if (graphicsPipeline == nullptr) {
        throw std::runtime_error("Graphics pipeline is null!");
    }
    return graphicsPipeline;
}

const vk::raii::PipelineLayout& MyPipeline::getPipelineLayout() const {
    if (pipelineLayout == nullptr) {
        throw std::runtime_error("Pipeline layout is null!");
    }
    return pipelineLayout;
}

vk::raii::ShaderModule MyPipeline::createShaderModule(
    const vk::raii::Device& device,
    const std::vector<char>& code) {
    vk::ShaderModuleCreateInfo createInfo{
        .codeSize = code.size() * sizeof(char),
        .pCode    = reinterpret_cast<const uint32_t*>(code.data())
    };
    return vk::raii::ShaderModule{device, createInfo};
}

std::vector<char> MyPipeline::readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("failed to open file: " + filename);
    }
    std::vector<char> buffer(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
    file.close();
    return buffer;
}