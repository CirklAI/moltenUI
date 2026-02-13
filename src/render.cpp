#include "render.hpp"
#include "components/input.hpp"
#include "font.hpp"
#include "init.hpp"
#include "shader.hpp"

namespace Render {
std::unique_ptr<Shader> rectShader = nullptr;
std::unique_ptr<FontRenderer> fontRenderer = nullptr;

struct RectUBO {
	glm::vec2 pos;
	glm::vec2 size;
	glm::vec4 color;
	glm::vec2 resolution;
};

const int MAX_FRAMES = 3;
const int MAX_RECTS_PER_FRAME = 256;

VkBuffer uniformBuffers[MAX_FRAMES][MAX_RECTS_PER_FRAME];
VmaAllocation uniformAllocs[MAX_FRAMES][MAX_RECTS_PER_FRAME];
void *uniformMapped[MAX_FRAMES][MAX_RECTS_PER_FRAME];
VkDescriptorPool descPool;
VkDescriptorSet descriptorSets[MAX_FRAMES][MAX_RECTS_PER_FRAME];
int rectCounters[MAX_FRAMES] = {0, 0, 0};

void init_resources() {
	rectShader = std::make_unique<Shader>("shaders/rect");
	fontRenderer = std::make_unique<FontRenderer>("assets/fonts/GoogleSans-Regular.ttf", 24);

	for(int frame = 0; frame < MAX_FRAMES; frame++) {
		for(int i = 0; i < MAX_RECTS_PER_FRAME; i++) {
			VkBufferCreateInfo bci{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			                       .size = sizeof(RectUBO),
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

	VkDescriptorPoolSize poolSize{.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	                              .descriptorCount = MAX_FRAMES * MAX_RECTS_PER_FRAME};
	VkDescriptorPoolCreateInfo pci{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
	                               .maxSets = MAX_FRAMES * MAX_RECTS_PER_FRAME,
	                               .poolSizeCount = 1,
	                               .pPoolSizes = &poolSize};
	vkCreateDescriptorPool(Init::device, &pci, nullptr, &descPool);

	for(int frame = 0; frame < MAX_FRAMES; frame++) {
		for(int i = 0; i < MAX_RECTS_PER_FRAME; i++) {
			VkDescriptorSetLayout layout = rectShader->descSetLayout;
			VkDescriptorSetAllocateInfo ai{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			                               .descriptorPool = descPool,
			                               .descriptorSetCount = 1,
			                               .pSetLayouts = &layout};
			vkAllocateDescriptorSets(Init::device, &ai, &descriptorSets[frame][i]);

			VkDescriptorBufferInfo bufferInfo{
			    .buffer = uniformBuffers[frame][i], .offset = 0, .range = sizeof(RectUBO)};
			VkWriteDescriptorSet write{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			                           .dstSet = descriptorSets[frame][i],
			                           .dstBinding = 0,
			                           .descriptorCount = 1,
			                           .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			                           .pBufferInfo = &bufferInfo};
			vkUpdateDescriptorSets(Init::device, 1, &write, 0, nullptr);
		}
	}
}

void draw_frame(std::function<void()> callback) {
	if(!rectShader) init_resources();

	vkWaitForFences(Init::device, 1, &Init::inFlightFences[Init::currentFrame], VK_TRUE, UINT64_MAX);
	uint32_t imageIndex;
	VkResult res =
	    vkAcquireNextImageKHR(Init::device, Init::vkbSwapchain.swapchain, UINT64_MAX,
	                          Init::imageAvailableSemaphores[Init::currentFrame], VK_NULL_HANDLE, &imageIndex);
	if(res == VK_ERROR_OUT_OF_DATE_KHR) return;
	vkResetFences(Init::device, 1, &Init::inFlightFences[Init::currentFrame]);
	vkResetCommandBuffer(Init::commandBuffers[Init::currentFrame], 0);

	VkCommandBuffer cmd = Init::commandBuffers[Init::currentFrame];
	VkCommandBufferBeginInfo bi{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
	vkBeginCommandBuffer(cmd, &bi);

	VkClearValue clearColor = {{{0.1f, 0.1f, 0.11f, 1.0f}}};
	VkRenderPassBeginInfo rp{.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
	                         .renderPass = Init::renderPass,
	                         .framebuffer = Init::framebuffers[imageIndex],
	                         .renderArea = {{0, 0}, Init::vkbSwapchain.extent},
	                         .clearValueCount = 1,
	                         .pClearValues = &clearColor};
	vkCmdBeginRenderPass(cmd, &rp, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport = {0, 0, (float)Init::vkbSwapchain.extent.width, (float)Init::vkbSwapchain.extent.height, 0, 1};
	vkCmdSetViewport(cmd, 0, 1, &viewport);
	VkRect2D scissor = {{0, 0}, Init::vkbSwapchain.extent};
	vkCmdSetScissor(cmd, 0, 1, &scissor);

	for(int i = 0; i < MAX_FRAMES; i++) {
		rectCounters[i] = 0;
	}
	fontRenderer->reset_char_counter();

	callback();

	vkCmdEndRenderPass(cmd);
	vkEndCommandBuffer(cmd);

	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSubmitInfo si{.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
	                .waitSemaphoreCount = 1,
	                .pWaitSemaphores = &Init::imageAvailableSemaphores[Init::currentFrame],
	                .pWaitDstStageMask = &waitStage,
	                .commandBufferCount = 1,
	                .pCommandBuffers = &cmd,
	                .signalSemaphoreCount = 1,
	                .pSignalSemaphores = &Init::renderFinishedSemaphores[Init::currentFrame]};
	vkQueueSubmit(Init::graphicsQueue, 1, &si, Init::inFlightFences[Init::currentFrame]);

	VkPresentInfoKHR pi{.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
	                    .waitSemaphoreCount = 1,
	                    .pWaitSemaphores = &Init::renderFinishedSemaphores[Init::currentFrame],
	                    .swapchainCount = 1,
	                    .pSwapchains = &Init::vkbSwapchain.swapchain,
	                    .pImageIndices = &imageIndex};
	vkQueuePresentKHR(Init::presentQueue, &pi);
	Init::currentFrame = (Init::currentFrame + 1) % Init::swapchainImages.size();
}

void draw_rect(float x, float y, float w, float h, glm::vec4 color) {
	VkCommandBuffer cmd = Init::commandBuffers[Init::currentFrame];
	int frame = Init::currentFrame;

	if(rectCounters[frame] >= MAX_RECTS_PER_FRAME) return;

	int idx = rectCounters[frame];

	rectShader->use(cmd);
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, rectShader->layout, 0, 1, &descriptorSets[frame][idx],
	                        0, nullptr);

	glm::vec2 scale = Input::get_content_scale();
	RectUBO *ubo = static_cast<RectUBO *>(uniformMapped[frame][idx]);
	ubo->pos = {x * scale.x, y * scale.y};
	ubo->size = {w * scale.x, h * scale.y};
	ubo->color = color;
	ubo->resolution = {(float)Init::vkbSwapchain.extent.width, (float)Init::vkbSwapchain.extent.height};

	vkCmdDraw(cmd, 6, 1, 0, 0);
	rectCounters[frame]++;
}

void set_scissor(float x, float y, float w, float h) {
	VkCommandBuffer cmd = Init::commandBuffers[Init::currentFrame];
	glm::vec2 scale = Input::get_content_scale();

	VkViewport viewport = {0, 0, (float)Init::vkbSwapchain.extent.width, (float)Init::vkbSwapchain.extent.height, 0, 1};
	vkCmdSetViewport(cmd, 0, 1, &viewport);

	VkRect2D scissor = {.offset = {(int32_t)(x * scale.x), (int32_t)(y * scale.y)},
	                    .extent = {(uint32_t)(w * scale.x), (uint32_t)(h * scale.y)}};
	vkCmdSetScissor(cmd, 0, 1, &scissor);
}

void reset_scissor() {
	VkCommandBuffer cmd = Init::commandBuffers[Init::currentFrame];

	VkViewport viewport = {0, 0, (float)Init::vkbSwapchain.extent.width, (float)Init::vkbSwapchain.extent.height, 0, 1};
	vkCmdSetViewport(cmd, 0, 1, &viewport);

	VkRect2D scissor = {.offset = {0, 0}, .extent = Init::vkbSwapchain.extent};
	vkCmdSetScissor(cmd, 0, 1, &scissor);
}

void cleanup() {
	vkDeviceWaitIdle(Init::device);
	fontRenderer.reset();
	rectShader.reset();
	vkDestroyDescriptorPool(Init::device, descPool, nullptr);
	for(int frame = 0; frame < MAX_FRAMES; frame++) {
		for(int i = 0; i < MAX_RECTS_PER_FRAME; i++) {
			vmaDestroyBuffer(Init::allocator, uniformBuffers[frame][i], uniformAllocs[frame][i]);
		}
	}
}
} // namespace Render
