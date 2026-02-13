#include "shader.hpp"
#include "init.hpp"
#include <fstream>
#include <stdexcept>
#include <vector>

Shader::Shader(const std::string &shaderPath) {
	auto loadCode = [](const std::string &path) {
		std::ifstream file(path, std::ios::ate | std::ios::binary);
		if(!file.is_open()) throw std::runtime_error("Failed to open: " + path);

		size_t fileSize = (size_t)file.tellg();
		if(fileSize == 0) throw std::runtime_error("Shader file is empty: " + path);

		std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
		file.seekg(0);
		file.read(reinterpret_cast<char *>(buffer.data()), fileSize);
		file.close();
		return buffer;
	};

	auto createMod = [&](const std::vector<uint32_t> &code) {
		VkShaderModuleCreateInfo ci{.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
		ci.codeSize = code.size() * sizeof(uint32_t);
		ci.pCode = code.data();
		VkShaderModule mod;
		if(vkCreateShaderModule(Init::device, &ci, nullptr, &mod) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create shader module");
		}
		return mod;
	};

	auto code = loadCode(shaderPath + ".spv");
	VkShaderModule shaderMod = createMod(code);

	VkPipelineShaderStageCreateInfo stages[2] = {{.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
	                                              .stage = VK_SHADER_STAGE_VERTEX_BIT,
	                                              .module = shaderMod,
	                                              .pName = "vs_main"},
	                                             {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
	                                              .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
	                                              .module = shaderMod,
	                                              .pName = "fs_main"}};

	VkDescriptorSetLayoutBinding uboBinding{.binding = 0,
	                                        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	                                        .descriptorCount = 1,
	                                        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT};

	VkDescriptorSetLayoutCreateInfo descLayoutInfo{
	    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, .bindingCount = 1, .pBindings = &uboBinding};
	vkCreateDescriptorSetLayout(Init::device, &descLayoutInfo, nullptr, &descSetLayout);

	VkPipelineLayoutCreateInfo lci{.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
	                               .setLayoutCount = 1,
	                               .pSetLayouts = &descSetLayout,
	                               .pushConstantRangeCount = 0,
	                               .pPushConstantRanges = nullptr};
	vkCreatePipelineLayout(Init::device, &lci, nullptr, &layout);

	VkPipelineVertexInputStateCreateInfo vi{.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
	VkPipelineInputAssemblyStateCreateInfo ia{.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
	                                          .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};

	VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	VkPipelineDynamicStateCreateInfo dynamicState{.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
	                                              .dynamicStateCount = 2,
	                                              .pDynamicStates = dynamicStates};

	VkPipelineViewportStateCreateInfo vps{
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, .viewportCount = 1, .scissorCount = 1};
	VkPipelineRasterizationStateCreateInfo rs{.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
	                                          .cullMode = VK_CULL_MODE_NONE,
	                                          .lineWidth = 1.0f};
	VkPipelineMultisampleStateCreateInfo ms{.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
	                                        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT};
	VkPipelineColorBlendAttachmentState cba{.colorWriteMask = 0xF};
	VkPipelineColorBlendStateCreateInfo cbs{
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, .attachmentCount = 1, .pAttachments = &cba};

	VkGraphicsPipelineCreateInfo gpi{.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
	                                 .stageCount = 2,
	                                 .pStages = stages,
	                                 .pVertexInputState = &vi,
	                                 .pInputAssemblyState = &ia,
	                                 .pViewportState = &vps,
	                                 .pRasterizationState = &rs,
	                                 .pMultisampleState = &ms,
	                                 .pColorBlendState = &cbs,
	                                 .pDynamicState = &dynamicState,
	                                 .layout = layout,
	                                 .renderPass = Init::renderPass};

	VkResult result = vkCreateGraphicsPipelines(Init::device, nullptr, 1, &gpi, nullptr, &pipeline);
	if(result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create graphics pipeline!");
	}

	vkDestroyShaderModule(Init::device, shaderMod, nullptr);
}

Shader::~Shader() {
	vkDestroyPipeline(Init::device, pipeline, nullptr);
	vkDestroyPipelineLayout(Init::device, layout, nullptr);
	vkDestroyDescriptorSetLayout(Init::device, descSetLayout, nullptr);
}

void Shader::use(VkCommandBuffer cmd) {
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}
