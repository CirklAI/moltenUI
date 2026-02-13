#include "font.hpp"
#include "components/input.hpp"
#include "init.hpp"
#include <fstream>
#include <vector>

struct TextUBO {
	glm::vec2 pos;
	glm::vec2 size;
	glm::vec4 color;
	glm::vec4 uvRect;
	glm::vec2 resolution;
};

const int MAX_FRAMES = 3;
const int MAX_CHARS_PER_FRAME = 256;

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

	for(int frame = 0; frame < MAX_FRAMES; frame++) {
		for(int i = 0; i < MAX_CHARS_PER_FRAME; i++) {
			VkBufferCreateInfo bci{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			                       .size = sizeof(TextUBO),
			                       .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT};
			VmaAllocationCreateInfo aci{.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
			                                     VMA_ALLOCATION_CREATE_MAPPED_BIT,
			                            .usage = VMA_MEMORY_USAGE_AUTO};
			VmaAllocationInfo allocInfo;
			vmaCreateBuffer(Init::allocator, &bci, &aci, &uniformBuffers[frame][i], &uniformAllocs[frame][i],
			                &allocInfo);
			uniformMapped[frame][i] = allocInfo.pMappedData;
		}
	}

	VkDescriptorSetLayoutBinding bindings[3] = {{.binding = 0,
	                                             .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	                                             .descriptorCount = 1,
	                                             .stageFlags = VK_SHADER_STAGE_VERTEX_BIT},
	                                            {.binding = 1,
	                                             .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
	                                             .descriptorCount = 1,
	                                             .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT},
	                                            {.binding = 2,
	                                             .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
	                                             .descriptorCount = 1,
	                                             .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT}};

	VkDescriptorSetLayoutCreateInfo dsci{
	    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, .bindingCount = 3, .pBindings = bindings};
	vkCreateDescriptorSetLayout(Init::device, &dsci, nullptr, &descLayout);

	VkPipelineLayoutCreateInfo plci{.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
	                                .setLayoutCount = 1,
	                                .pSetLayouts = &descLayout,
	                                .pushConstantRangeCount = 0,
	                                .pPushConstantRanges = nullptr};
	vkCreatePipelineLayout(Init::device, &plci, nullptr, &pipelineLayout);

	auto loadCode = [](const std::string &path) {
		std::ifstream file(path, std::ios::ate | std::ios::binary);
		if(!file.is_open()) throw std::runtime_error("Failed to open: " + path);
		size_t fileSize = (size_t)file.tellg();
		std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
		file.seekg(0);
		file.read(reinterpret_cast<char *>(buffer.data()), fileSize);
		return buffer;
	};

	auto code = loadCode("shaders/text.spv");

	VkShaderModuleCreateInfo ci{.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
	                            .codeSize = code.size() * sizeof(uint32_t),
	                            .pCode = code.data()};
	VkShaderModule shaderMod;
	vkCreateShaderModule(Init::device, &ci, nullptr, &shaderMod);

	VkPipelineShaderStageCreateInfo stages[2] = {{.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
	                                              .stage = VK_SHADER_STAGE_VERTEX_BIT,
	                                              .module = shaderMod,
	                                              .pName = "vs_main"},
	                                             {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
	                                              .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
	                                              .module = shaderMod,
	                                              .pName = "fs_main"}};

	VkPipelineVertexInputStateCreateInfo vi{.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
	VkPipelineInputAssemblyStateCreateInfo ia{.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
	                                          .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};

	// Dynamic viewport and scissor for text clipping
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
	                                 .pDynamicState = &dynamicState,
	                                 .layout = pipelineLayout,
	                                 .renderPass = Init::renderPass};
	VkResult result = vkCreateGraphicsPipelines(Init::device, nullptr, 1, &gpi, nullptr, &pipeline);
	if(result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create text graphics pipeline!");
	}

	vkDestroyShaderModule(Init::device, shaderMod, nullptr);

	VkDescriptorPoolSize poolSizes[3] = {
	    {.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = MAX_FRAMES * MAX_CHARS_PER_FRAME},
	    {.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, .descriptorCount = MAX_FRAMES * MAX_CHARS_PER_FRAME},
	    {.type = VK_DESCRIPTOR_TYPE_SAMPLER, .descriptorCount = MAX_FRAMES * MAX_CHARS_PER_FRAME}};
	VkDescriptorPoolCreateInfo pci{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
	                               .maxSets = MAX_FRAMES * MAX_CHARS_PER_FRAME,
	                               .poolSizeCount = 3,
	                               .pPoolSizes = poolSizes};
	vkCreateDescriptorPool(Init::device, &pci, nullptr, &descPool);

	for(int frame = 0; frame < MAX_FRAMES; frame++) {
		for(int i = 0; i < MAX_CHARS_PER_FRAME; i++) {
			VkDescriptorSetLayout layout = descLayout;
			VkDescriptorSetAllocateInfo ai{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			                               .descriptorPool = descPool,
			                               .descriptorSetCount = 1,
			                               .pSetLayouts = &layout};
			vkAllocateDescriptorSets(Init::device, &ai, &descriptorSets[frame][i]);

			VkDescriptorBufferInfo bufferInfo{
			    .buffer = uniformBuffers[frame][i], .offset = 0, .range = sizeof(TextUBO)};

			VkDescriptorImageInfo imageInfo{.imageView = atlasView,
			                                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

			VkDescriptorImageInfo samplerInfo{.sampler = atlasSampler};

			VkWriteDescriptorSet writes[3] = {{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			                                   .dstSet = descriptorSets[frame][i],
			                                   .dstBinding = 0,
			                                   .descriptorCount = 1,
			                                   .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			                                   .pBufferInfo = &bufferInfo},
			                                  {.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			                                   .dstSet = descriptorSets[frame][i],
			                                   .dstBinding = 1,
			                                   .descriptorCount = 1,
			                                   .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
			                                   .pImageInfo = &imageInfo},
			                                  {.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			                                   .dstSet = descriptorSets[frame][i],
			                                   .dstBinding = 2,
			                                   .descriptorCount = 1,
			                                   .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
			                                   .pImageInfo = &samplerInfo}};
			vkUpdateDescriptorSets(Init::device, 3, writes, 0, nullptr);
		}
	}
}

FontRenderer::~FontRenderer() {
	vkDestroySampler(Init::device, atlasSampler, nullptr);
	vkDestroyImageView(Init::device, atlasView, nullptr);
	vmaDestroyImage(Init::allocator, atlasImage, atlasAllocation);
	vkDestroyDescriptorPool(Init::device, descPool, nullptr);
	vkDestroyDescriptorSetLayout(Init::device, descLayout, nullptr);
	vkDestroyPipeline(Init::device, pipeline, nullptr);
	vkDestroyPipelineLayout(Init::device, pipelineLayout, nullptr);
	for(int frame = 0; frame < MAX_FRAMES; frame++) {
		for(int i = 0; i < MAX_CHARS_PER_FRAME; i++) {
			vmaDestroyBuffer(Init::allocator, uniformBuffers[frame][i], uniformAllocs[frame][i]);
		}
	}
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

	const uint32_t padding = 2;
	uint32_t atlasW = 0, atlasH = 0;
	for(unsigned char c = 32; c < 128; c++) {
		FT_Load_Char(face, c, FT_LOAD_RENDER);
		atlasW += face->glyph->bitmap.width + padding;
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
	lineHeight = 0;
	for(unsigned char c = 32; c < 128; c++) {
		FT_Load_Char(face, c, FT_LOAD_RENDER);
		auto &bitmap = face->glyph->bitmap;
		for(uint32_t row = 0; row < bitmap.rows; row++) {
			memcpy(pixels + (row * atlasW) + xOffset, bitmap.buffer + (row * bitmap.width), bitmap.width);
		}
		glyphs[c] = {glm::vec2(bitmap.width, bitmap.rows), glm::vec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
		             (uint32_t)face->glyph->advance.x >> 6,
		             glm::vec4((float)xOffset / atlasW, 0, (float)bitmap.width / atlasW, (float)bitmap.rows / atlasH)};
		xOffset += bitmap.width + padding;
		lineHeight = std::max(lineHeight, (float)bitmap.rows);
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

	float x = pos.x;
	int frame = Init::currentFrame;
	glm::vec2 scale = Input::get_content_scale();

	for(char c : text) {
		if(charCounters[frame] >= MAX_CHARS_PER_FRAME) break;
		if(glyphs.find(c) == glyphs.end()) continue;
		Glyph &g = glyphs[c];
		float xpos = x + g.bearing.x;
		float ypos = pos.y - g.bearing.y;

		int idx = charCounters[frame];
		TextUBO *ubo = static_cast<TextUBO *>(uniformMapped[frame][idx]);
		ubo->pos = {xpos * scale.x, ypos * scale.y};
		ubo->size = {g.size.x * scale.x, g.size.y * scale.y};
		ubo->color = color;
		ubo->uvRect = g.uvRect;
		ubo->resolution = {(float)Init::vkbSwapchain.extent.width, (float)Init::vkbSwapchain.extent.height};

		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[frame][idx],
		                        0, nullptr);
		vkCmdDraw(cmd, 6, 1, 0, 0);
		x += g.advance;
		charCounters[frame]++;
	}
}

void FontRenderer::reset_char_counter() {
	for(int i = 0; i < MAX_FRAMES; i++) {
		charCounters[i] = 0;
	}
}

float FontRenderer::measure_text(const std::string &text) {
	float width = 0;
	for(char c : text) {
		auto it = glyphs.find(c);
		if(it != glyphs.end()) {
			width += it->second.advance;
		}
	}
	return width;
}

const Glyph *FontRenderer::get_glyph(char c) const {
	auto it = glyphs.find(c);
	if(it != glyphs.end()) {
		return &it->second;
	}
	return nullptr;
}
