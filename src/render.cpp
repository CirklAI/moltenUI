#include "render.hpp"
#include "components/input.hpp"
#include "font.hpp"
#include "init.hpp"
#include "ui.hpp"

namespace Render {
std::unique_ptr<Shader> rectShader = nullptr;
std::unique_ptr<FontRenderer> fontRenderer = nullptr;

void draw_frame() {
	if(!rectShader) rectShader = std::make_unique<Shader>("shaders/rect", "shaders/rect");
	if(!fontRenderer) fontRenderer = std::make_unique<FontRenderer>("assets/fonts/SF-Pro-Rounded-Regular.otf", 24);

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

	Input::update();
	glfwPollEvents();

	rectShader->use(cmd);

	static std::string buffer = "Type here...";
	UI::Text("MoltenUI Debug", {20, 30}, {0.4f, 0.6f, 1.0f, 1.0f});

	if(UI::Button(0, {20, 60}, {100, 35}, "Clear")) {
		buffer = "";
	}

	UI::InputField(1, buffer, {20, 110}, {250, 40});
	UI::Text("Status: Active", {20, 170}, {0.5f, 0.5f, 0.5f, 1.0f});

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

void cleanup() {
	vkDeviceWaitIdle(Init::device);
	fontRenderer.reset();
	rectShader.reset();
}
} // namespace Render
