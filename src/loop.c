#include "loop.h"
#include "font.h"
#include <GLFW/glfw3.h>
#include <string.h>

void molten_loop(VkDevice dev, VkFence *fens, uint32_t *cur_frame, VkSwapchainKHR swap, VkSemaphore *semps_img_avl,
                 VkFence *fens_img, VkCommandBuffer *cmd_buffers, VkSemaphore *semps_rend_fin, VkQueue q_pres,
                 VkQueue q_graph, uint32_t max_frames, FontAtlas *atlas, void *mapped_vbo_data,
                 VkRenderPass render_pass, VkFramebuffer *framebuffers, VkPipeline pipeline, VkPipelineLayout layout,
                 VkExtent2D extent, VkBuffer vertex_buffer, VkDescriptorSet descriptor_set) {

	glfwPollEvents();

	vkWaitForFences(dev, 1, &(fens[*cur_frame]), VK_TRUE, UINT64_MAX);

	const char *message = "Vulkan C99 OTF";
	update_text_buffer(atlas, message, 50.0f, 50.0f, mapped_vbo_data);

	uint32_t img_index = 0;
	VkResult result =
	    vkAcquireNextImageKHR(dev, swap, UINT64_MAX, semps_img_avl[*cur_frame], VK_NULL_HANDLE, &img_index);

	if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) return;

	if(fens_img[img_index] != VK_NULL_HANDLE) {
		vkWaitForFences(dev, 1, &(fens_img[img_index]), VK_TRUE, UINT64_MAX);
	}
	fens_img[img_index] = fens[*cur_frame];

	vkResetCommandBuffer(cmd_buffers[img_index], 0);
	VkCommandBufferBeginInfo begin_info = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
	vkBeginCommandBuffer(cmd_buffers[img_index], &begin_info);

	VkClearValue clear_color = {{{0.1f, 0.1f, 0.1f, 1.0f}}};
	VkRenderPassBeginInfo rp_info = {.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
	                                 .renderPass = render_pass,
	                                 .framebuffer = framebuffers[img_index],
	                                 .renderArea.extent = extent,
	                                 .clearValueCount = 1,
	                                 .pClearValues = &clear_color};

	vkCmdBeginRenderPass(cmd_buffers[img_index], &rp_info, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(cmd_buffers[img_index], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

	float screen_res[2] = {(float)extent.width, (float)extent.height};
	vkCmdPushConstants(cmd_buffers[img_index], layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(screen_res), screen_res);

	VkDeviceSize offsets[] = {0};
	vkCmdBindVertexBuffers(cmd_buffers[img_index], 0, 1, &vertex_buffer, offsets);
	vkCmdBindDescriptorSets(cmd_buffers[img_index], VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &descriptor_set, 0,
	                        NULL);

	vkCmdDraw(cmd_buffers[img_index], (uint32_t)strlen(message) * 6, 1, 0, 0);
	vkCmdEndRenderPass(cmd_buffers[img_index]);
	vkEndCommandBuffer(cmd_buffers[img_index]);

	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	VkSubmitInfo sub_info = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
	                         .waitSemaphoreCount = 1,
	                         .pWaitSemaphores = &semps_img_avl[*cur_frame],
	                         .pWaitDstStageMask = waitStages,
	                         .commandBufferCount = 1,
	                         .pCommandBuffers = &cmd_buffers[img_index],
	                         .signalSemaphoreCount = 1,
	                         .pSignalSemaphores = &semps_rend_fin[img_index]};

	vkResetFences(dev, 1, &(fens[*cur_frame]));
	vkQueueSubmit(q_graph, 1, &sub_info, fens[*cur_frame]);

	VkPresentInfoKHR pres_info = {.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
	                              .waitSemaphoreCount = 1,
	                              .pWaitSemaphores = &semps_rend_fin[img_index],
	                              .swapchainCount = 1,
	                              .pSwapchains = &swap,
	                              .pImageIndices = &img_index};
	vkQueuePresentKHR(q_pres, &pres_info);

	*cur_frame = (*cur_frame + 1) % max_frames;
}
