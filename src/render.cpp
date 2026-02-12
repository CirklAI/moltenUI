#include "render.hpp"
#include "init.hpp"
#include "shader.hpp"
#include <memory>

namespace Render {
std::unique_ptr<Shader> rectShader = nullptr;

void draw_frame() {
	if(!rectShader) {
		rectShader = std::make_unique<Shader>("shaders/rect.vert", "shaders/rect.frag");
	}

	vkWaitForFences(Init::device, 1, &Init::inFlightFences[Init::currentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	VkResult result =
	    vkAcquireNextImageKHR(Init::device, Init::vkbSwapchain.swapchain, UINT64_MAX,
	                          Init::imageAvailableSemaphores[Init::currentFrame], VK_NULL_HANDLE, &imageIndex);

	if(result == VK_ERROR_OUT_OF_DATE_KHR) return;

	vkResetFences(Init::device, 1, &Init::inFlightFences[Init::currentFrame]);
	vkResetCommandBuffer(Init::commandBuffers[Init::currentFrame], 0);

	VkCommandBufferBeginInfo bi{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
	vkBeginCommandBuffer(Init::commandBuffers[Init::currentFrame], &bi);

	VkClearValue clear = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
	VkRenderPassBeginInfo rp{.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
	                         .renderPass = Init::renderPass,
	                         .framebuffer = Init::framebuffers[imageIndex],
	                         .renderArea = {{0, 0}, Init::vkbSwapchain.extent},
	                         .clearValueCount = 1,
	                         .pClearValues = &clear};

	vkCmdBeginRenderPass(Init::commandBuffers[Init::currentFrame], &rp, VK_SUBPASS_CONTENTS_INLINE);

	rectShader->use(Init::commandBuffers[Init::currentFrame]);

	float pc[4] = {0.0f, 0.0f, 0.5f, 0.5f};
	vkCmdPushConstants(Init::commandBuffers[Init::currentFrame], rectShader->layout, VK_SHADER_STAGE_VERTEX_BIT, 0,
	                   sizeof(pc), pc);

	vkCmdDraw(Init::commandBuffers[Init::currentFrame], 6, 1, 0, 0);

	vkCmdEndRenderPass(Init::commandBuffers[Init::currentFrame]);
	vkEndCommandBuffer(Init::commandBuffers[Init::currentFrame]);

	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSubmitInfo si{.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
	                .waitSemaphoreCount = 1,
	                .pWaitSemaphores = &Init::imageAvailableSemaphores[Init::currentFrame],
	                .pWaitDstStageMask = &waitStage,
	                .commandBufferCount = 1,
	                .pCommandBuffers = &Init::commandBuffers[Init::currentFrame],
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

void cleanup() {
	rectShader.reset();
}

} // namespace Render
