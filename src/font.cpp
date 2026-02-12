#include "font.hpp"
#include "init.hpp"
#include <fstream>
#include <vector>

static std::vector<char> readFile(const std::string &filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	if(!file.is_open()) throw std::runtime_error("failed to open file!");
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	return buffer;
}

static VkShaderModule createModule(const std::vector<char> &code) {
	VkShaderModuleCreateInfo ci{.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
	                            .codeSize = code.size(),
	                            .pCode = reinterpret_cast<const uint32_t *>(code.data())};
	VkShaderModule mod;
	vkCreateShaderModule(Init::device, &ci, nullptr, &mod);
	return mod;
}

FontRenderer::FontRenderer(const std::string &fontPath, uint32_t fontSize) {
	create_atlas(fontPath, fontSize);

	VkDescriptorSetLayoutBinding binding{.binding = 0,
	                                     .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
	                                     .descriptorCount = 1,
	                                     .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT};
	VkDescriptorSetLayoutCreateInfo dsci{
	    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, .bindingCount = 1, .pBindings = &binding};
	vkCreateDescriptorSetLayout(Init::device, &dsci, nullptr, &descLayout);

	VkPushConstantRange push{.stageFlags = VK_SHADER_STAGE_VERTEX_BIT, .offset = 0, .size = 48};
	VkPipelineLayoutCreateInfo plci{.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
	                                .setLayoutCount = 1,
	                                .pSetLayouts = &descLayout,
	                                .pushConstantRangeCount = 1,
	                                .pPushConstantRanges = &push};
	vkCreatePipelineLayout(Init::device, &plci, nullptr, &pipelineLayout);

	// FIX: Load shaders from 'shaders' to match the build output
	auto vCode = readFile("shaders/text.vert.spv");
	auto fCode = readFile("shaders/text.frag.spv");
	VkShaderModule vMod = createModule(vCode);
	VkShaderModule fMod = createModule(fCode);

	VkPipelineShaderStageCreateInfo stages[2] = {{.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
	                                              .stage = VK_SHADER_STAGE_VERTEX_BIT,
	                                              .module = vMod,
	                                              .pName = "main"},
	                                             {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
	                                              .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
	                                              .module = fMod,
	                                              .pName = "main"}};

	VkPipelineVertexInputStateCreateInfo vi{.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
	VkPipelineInputAssemblyStateCreateInfo ia{.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
	                                          .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};
	VkViewport vp{0, 0, (float)Init::vkbSwapchain.extent.width, (float)Init::vkbSwapchain.extent.height, 0, 1};
	VkRect2D sc{{0, 0}, Init::vkbSwapchain.extent};
	VkPipelineViewportStateCreateInfo vps{.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
	                                      .viewportCount = 1,
	                                      .pViewports = &vp,
	                                      .scissorCount = 1,
	                                      .pScissors = &sc};
	VkPipelineRasterizationStateCreateInfo rs{.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
	                                          .cullMode = VK_CULL_MODE_NONE,
	                                          .lineWidth = 1.0f};
	VkPipelineMultisampleStateCreateInfo ms{.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
	                                        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT};
	VkPipelineColorBlendAttachmentState cba{.blendEnable = VK_TRUE,
	                                        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
	                                        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
	                                        .colorBlendOp = VK_BLEND_OP_ADD,
	                                        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
	                                        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
	                                        .alphaBlendOp = VK_BLEND_OP_ADD,
	                                        .colorWriteMask = 0xF};
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
	                                 .layout = pipelineLayout,
	                                 .renderPass = Init::renderPass};
	VkResult result = vkCreateGraphicsPipelines(Init::device, nullptr, 1, &gpi, nullptr, &pipeline);
	if(result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create text graphics pipeline!");
	}

	vkDestroyShaderModule(Init::device, vMod, nullptr);
	vkDestroyShaderModule(Init::device, fMod, nullptr);

	VkDescriptorPoolSize poolSize{.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1};
	VkDescriptorPoolCreateInfo pci{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
	                               .maxSets = 1,
	                               .poolSizeCount = 1,
	                               .pPoolSizes = &poolSize};
	vkCreateDescriptorPool(Init::device, &pci, nullptr, &descPool);

	VkDescriptorSetAllocateInfo ai{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
	                               .descriptorPool = descPool,
	                               .descriptorSetCount = 1,
	                               .pSetLayouts = &descLayout};
	vkAllocateDescriptorSets(Init::device, &ai, &descriptorSet);

	VkDescriptorImageInfo imageInfo{
	    .sampler = atlasSampler, .imageView = atlasView, .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
	VkWriteDescriptorSet write{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
	                           .dstSet = descriptorSet,
	                           .dstBinding = 0,
	                           .descriptorCount = 1,
	                           .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
	                           .pImageInfo = &imageInfo};
	vkUpdateDescriptorSets(Init::device, 1, &write, 0, nullptr);
}

FontRenderer::~FontRenderer() {
	vkDestroySampler(Init::device, atlasSampler, nullptr);
	vkDestroyImageView(Init::device, atlasView, nullptr);
	vmaDestroyImage(Init::allocator, atlasImage, atlasAllocation);
	vkDestroyDescriptorPool(Init::device, descPool, nullptr);
	vkDestroyDescriptorSetLayout(Init::device, descLayout, nullptr);
	vkDestroyPipeline(Init::device, pipeline, nullptr);
	vkDestroyPipelineLayout(Init::device, pipelineLayout, nullptr);
}

void FontRenderer::create_atlas(const std::string &path, uint32_t fontSize) {
	FT_Library ft;
	if(FT_Init_FreeType(&ft)) {
		throw std::runtime_error("Could not init FreeType Library");
	}

	FT_Face face;
	if(FT_New_Face(ft, path.c_str(), 0, &face)) {
		throw std::runtime_error("Failed to load font at: " + path);
	}

	FT_Set_Pixel_Sizes(face, 0, fontSize);

	uint32_t atlasW = 0, atlasH = 0;
	for(unsigned char c = 32; c < 128; c++) {
		FT_Load_Char(face, c, FT_LOAD_RENDER);
		atlasW += face->glyph->bitmap.width;
		atlasH = std::max(atlasH, (uint32_t)face->glyph->bitmap.rows);
	}

	VkBuffer stagingBuffer;
	VmaAllocation stagingAlloc;
	VkBufferCreateInfo bci{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
	                       .size = (VkDeviceSize)atlasW * atlasH,
	                       .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT};
	VmaAllocationCreateInfo aci{.usage = VMA_MEMORY_USAGE_CPU_ONLY};
	vmaCreateBuffer(Init::allocator, &bci, &aci, &stagingBuffer, &stagingAlloc, nullptr);

	void *data;
	vmaMapMemory(Init::allocator, stagingAlloc, &data);
	uint8_t *pixels = (uint8_t *)data;
	memset(pixels, 0, atlasW * atlasH);

	uint32_t xOffset = 0;
	for(unsigned char c = 32; c < 128; c++) {
		FT_Load_Char(face, c, FT_LOAD_RENDER);
		auto &bitmap = face->glyph->bitmap;
		for(uint32_t row = 0; row < bitmap.rows; row++) {
			memcpy(pixels + (row * atlasW) + xOffset, bitmap.buffer + (row * bitmap.width), bitmap.width);
		}
		glyphs[c] = {glm::vec2(bitmap.width, bitmap.rows), glm::vec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
		             (uint32_t)face->glyph->advance.x >> 6,
		             glm::vec4((float)xOffset / atlasW, 0, (float)bitmap.width / atlasW, (float)bitmap.rows / atlasH)};
		xOffset += bitmap.width;
	}
	vmaUnmapMemory(Init::allocator, stagingAlloc);

	VkImageCreateInfo ici{.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
	                      .imageType = VK_IMAGE_TYPE_2D,
	                      .format = VK_FORMAT_R8_UNORM,
	                      .extent = {atlasW, atlasH, 1},
	                      .mipLevels = 1,
	                      .arrayLayers = 1,
	                      .samples = VK_SAMPLE_COUNT_1_BIT,
	                      .tiling = VK_IMAGE_TILING_OPTIMAL,
	                      .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT};
	VmaAllocationCreateInfo iaci{.usage = VMA_MEMORY_USAGE_GPU_ONLY};
	vmaCreateImage(Init::allocator, &ici, &iaci, &atlasImage, &atlasAllocation, nullptr);

	VkCommandBufferAllocateInfo cbai{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
	                                 .commandPool = Init::commandPool,
	                                 .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
	                                 .commandBufferCount = 1};
	VkCommandBuffer tempCmd;
	vkAllocateCommandBuffers(Init::device, &cbai, &tempCmd);
	VkCommandBufferBeginInfo cbi{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	                             .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};
	vkBeginCommandBuffer(tempCmd, &cbi);

	VkImageMemoryBarrier barrier{.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
	                             .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
	                             .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	                             .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	                             .image = atlasImage,
	                             .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};
	vkCmdPipelineBarrier(tempCmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
	                     nullptr, 1, &barrier);

	VkBufferImageCopy region{.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
	                         .imageExtent = {atlasW, atlasH, 1}};
	vkCmdCopyBufferToImage(tempCmd, stagingBuffer, atlasImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier(tempCmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr,
	                     0, nullptr, 1, &barrier);
	vkEndCommandBuffer(tempCmd);
	VkSubmitInfo sub{.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO, .commandBufferCount = 1, .pCommandBuffers = &tempCmd};
	vkQueueSubmit(Init::graphicsQueue, 1, &sub, VK_NULL_HANDLE);
	vkQueueWaitIdle(Init::graphicsQueue);
	vkFreeCommandBuffers(Init::device, Init::commandPool, 1, &tempCmd);

	VkImageViewCreateInfo vci{.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
	                          .image = atlasImage,
	                          .viewType = VK_IMAGE_VIEW_TYPE_2D,
	                          .format = VK_FORMAT_R8_UNORM,
	                          .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};
	vkCreateImageView(Init::device, &vci, nullptr, &atlasView);

	VkSamplerCreateInfo sci{.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
	                        .magFilter = VK_FILTER_LINEAR,
	                        .minFilter = VK_FILTER_LINEAR,
	                        .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
	                        .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
	                        .borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK};
	vkCreateSampler(Init::device, &sci, nullptr, &atlasSampler);

	vmaDestroyBuffer(Init::allocator, stagingBuffer, stagingAlloc);
	FT_Done_Face(face);
	FT_Done_FreeType(ft);
}

void FontRenderer::draw_text(VkCommandBuffer cmd, const std::string &text, glm::vec2 pos, glm::vec4 color) {
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
	float x = pos.x;
	for(char c : text) {
		if(glyphs.find(c) == glyphs.end()) continue;
		Glyph &g = glyphs[c];
		float xpos = x + g.bearing.x;
		float ypos = pos.y - g.bearing.y;
		struct {
			glm::vec2 p, s;
			glm::vec4 c, uv;
		} pc{{xpos, ypos}, g.size, color, g.uvRect};
		vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(pc), &pc);
		vkCmdDraw(cmd, 6, 1, 0, 0);
		x += g.advance;
	}
}
