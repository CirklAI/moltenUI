#include "pipeline.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PipelineContext create_graphics_pipeline(VkDevice dev, VkRenderPass render_pass, VkExtent2D extent) {
	PipelineContext ctx;

	VkPushConstantRange push_constant = {
	    .offset = 0, .size = sizeof(float) * 2, .stageFlags = VK_SHADER_STAGE_VERTEX_BIT};

	VkDescriptorSetLayoutBinding samplerLayoutBinding = {.binding = 1,
	                                                     .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
	                                                     .descriptorCount = 1,
	                                                     .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
	                                                     .pImmutableSamplers = NULL};

	VkDescriptorSetLayoutCreateInfo layoutInfo = {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
	                                              .bindingCount = 1,
	                                              .pBindings = &samplerLayoutBinding};

	if(vkCreateDescriptorSetLayout(dev, &layoutInfo, NULL, &ctx.descriptor_set_layout) != VK_SUCCESS) {
		printf("failed to create descriptor set layout!\n");
	}

	VkPipelineLayoutCreateInfo pipe_lay_cre_info = {.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
	                                                .setLayoutCount = 1,
	                                                .pSetLayouts = &ctx.descriptor_set_layout,
	                                                .pushConstantRangeCount = 1,
	                                                .pPushConstantRanges = &push_constant};

	if(vkCreatePipelineLayout(dev, &pipe_lay_cre_info, NULL, &ctx.layout) != VK_SUCCESS) {
		printf("failed to create pipeline layout!\n");
	}

	FILE *fp_vert = fopen("text.vert.spv", "rb");
	FILE *fp_frag = fopen("text.frag.spv", "rb");

	if(!fp_vert || !fp_frag) {
		printf("Error: Can't find 'text.vert.spv' or 'text.frag.spv'. Did you "
		       "compile them?\n");
		exit(1);
	}

	fseek(fp_vert, 0, SEEK_END);
	uint32_t vert_size = ftell(fp_vert);
	rewind(fp_vert);
	char *p_vert_code = malloc(vert_size);
	fread(p_vert_code, 1, vert_size, fp_vert);
	fclose(fp_vert);

	fseek(fp_frag, 0, SEEK_END);
	uint32_t frag_size = ftell(fp_frag);
	rewind(fp_frag);
	char *p_frag_code = malloc(frag_size);
	fread(p_frag_code, 1, frag_size, fp_frag);
	fclose(fp_frag);

	VkShaderModuleCreateInfo vert_info = {
	    .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, .codeSize = vert_size, .pCode = (uint32_t *)p_vert_code};

	VkShaderModuleCreateInfo frag_info = {
	    .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, .codeSize = frag_size, .pCode = (uint32_t *)p_frag_code};

	VkShaderModule vert_mod, frag_mod;
	vkCreateShaderModule(dev, &vert_info, NULL, &vert_mod);
	vkCreateShaderModule(dev, &frag_info, NULL, &frag_mod);

	VkPipelineShaderStageCreateInfo shaderStages[] = {{.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
	                                                   .stage = VK_SHADER_STAGE_VERTEX_BIT,
	                                                   .module = vert_mod,
	                                                   .pName = "main"},
	                                                  {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
	                                                   .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
	                                                   .module = frag_mod,
	                                                   .pName = "main"}};

	VkVertexInputBindingDescription bindingDescription = {.binding = 0,
	                                                      .stride = sizeof(float) * 4, // x, y, u, v
	                                                      .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};

	VkVertexInputAttributeDescription attributeDescriptions[2] = {
	    {.binding = 0, .location = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = 0},
	    {.binding = 0, .location = 1, .format = VK_FORMAT_R32G32_SFLOAT, .offset = 8}};

	VkPipelineVertexInputStateCreateInfo vert_input_info = {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
	    .vertexBindingDescriptionCount = 1,
	    .pVertexBindingDescriptions = &bindingDescription,
	    .vertexAttributeDescriptionCount = 2,
	    .pVertexAttributeDescriptions = attributeDescriptions};

	VkPipelineInputAssemblyStateCreateInfo input_asm_info = {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
	    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	    .primitiveRestartEnable = VK_FALSE};

	VkViewport viewport = {0.0f, 0.0f, (float)extent.width, (float)extent.height, 0.0f, 1.0f};
	VkRect2D scissor = {{0, 0}, extent};
	VkPipelineViewportStateCreateInfo vwp_state = {.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
	                                               .viewportCount = 1,
	                                               .pViewports = &viewport,
	                                               .scissorCount = 1,
	                                               .pScissors = &scissor};

	VkPipelineRasterizationStateCreateInfo rast_info = {.sType =
	                                                        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
	                                                    .depthClampEnable = VK_FALSE,
	                                                    .rasterizerDiscardEnable = VK_FALSE,
	                                                    .polygonMode = VK_POLYGON_MODE_FILL,
	                                                    .cullMode = VK_CULL_MODE_NONE, // both sides
	                                                    .frontFace = VK_FRONT_FACE_CLOCKWISE,
	                                                    .depthBiasEnable = VK_FALSE,
	                                                    .lineWidth = 1.0f};

	VkPipelineMultisampleStateCreateInfo multisample_info = {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
	    .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
	    .sampleShadingEnable = VK_FALSE};

	VkPipelineColorBlendAttachmentState color_blend = {.colorWriteMask =
	                                                       VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
	                                                       VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
	                                                   .blendEnable = VK_TRUE,
	                                                   .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
	                                                   .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
	                                                   .colorBlendOp = VK_BLEND_OP_ADD,
	                                                   .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
	                                                   .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
	                                                   .alphaBlendOp = VK_BLEND_OP_ADD};

	VkPipelineColorBlendStateCreateInfo blend_info = {.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
	                                                  .logicOpEnable = VK_FALSE,
	                                                  .attachmentCount = 1,
	                                                  .pAttachments = &color_blend};

	VkGraphicsPipelineCreateInfo pipelineInfo = {.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
	                                             .stageCount = 2,
	                                             .pStages = shaderStages,
	                                             .pVertexInputState = &vert_input_info,
	                                             .pInputAssemblyState = &input_asm_info,
	                                             .pViewportState = &vwp_state,
	                                             .pRasterizationState = &rast_info,
	                                             .pMultisampleState = &multisample_info,
	                                             .pColorBlendState = &blend_info,
	                                             .layout = ctx.layout,
	                                             .renderPass = render_pass,
	                                             .subpass = 0};

	if(vkCreateGraphicsPipelines(dev, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &ctx.pipeline) != VK_SUCCESS) {
		printf("failed to create graphics pipeline!\n");
	} else {
		printf("graphics pipeline created.\n");
	}

	vkDestroyShaderModule(dev, vert_mod, NULL);
	vkDestroyShaderModule(dev, frag_mod, NULL);
	free(p_vert_code);
	free(p_frag_code);

	return ctx;
}
