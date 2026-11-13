#ifndef MY_PIPELINE_H
#define MY_PIPELINE_H

#include <vulkan/vulkan_raii.hpp>
#include <string>
#include <vector>

class VulkanContext;

struct PipelineConfigInfo {
    std::string vertShaderPath;
    std::string fragShaderPath;

    std::vector<vk::VertexInputBindingDescription>   bindingDescriptions;
    std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
    vk::PipelineViewportStateCreateInfo      viewportState;
    vk::PipelineRasterizationStateCreateInfo rasterizer;
    vk::PipelineMultisampleStateCreateInfo   multisampling;
    vk::PipelineColorBlendAttachmentState    colorBlendAttachment;
    std::vector<vk::DynamicState>            dynamicStates;

    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
    std::vector<vk::PushConstantRange>   pushConstantRanges;

    vk::Format colorAttachmentFormat = vk::Format::eUndefined;

    static PipelineConfigInfo defaultConfig(vk::Format colorFormat);
};

class MyPipeline {
    public:
        MyPipeline(const VulkanContext& ctx, const PipelineConfigInfo& config);

        const vk::raii::Pipeline& getPipeline() const;
        const vk::raii::PipelineLayout& getPipelineLayout() const;

    private:
        vk::raii::PipelineLayout pipelineLayout   = nullptr;
        vk::raii::Pipeline       graphicsPipeline = nullptr;

        void create(const vk::raii::Device& device, const PipelineConfigInfo& config);

        [[nodiscard]] static vk::raii::ShaderModule createShaderModule(
            const vk::raii::Device& device,
            const std::vector<char>& code);

        static std::vector<char> readFile(const std::string& filename);
};

#endif // MY_PIPELINE_H