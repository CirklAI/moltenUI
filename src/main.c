// clang-format off
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "command.h"
#include "device.h"
#include "framebuffer.h"
#include "instance.h"
#include "loop.h"
#include "pipeline.h"
#include "renderpass.h"
#include "swapchain.h"
#include "sync.h"
#include "window.h"

#define VBO_SIZE 1024 * 64

uint32_t find_memory_type(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}
	return 0;
}

void submit_single_time_command(VkDevice device, VkCommandPool commandPool, VkQueue queue,
                                void (*record_func)(VkCommandBuffer, void *), void *user_data) {
	VkCommandBufferAllocateInfo allocInfo = {
	    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
	    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
	    .commandPool = commandPool,
	    .commandBufferCount = 1,
	};

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {
	    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};
	vkBeginCommandBuffer(commandBuffer, &beginInfo);
	record_func(commandBuffer, user_data);
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {
	    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
	    .commandBufferCount = 1,
	    .pCommandBuffers = &commandBuffer,
	};

	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);
	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

typedef struct {
	VkBuffer buffer;
	VkImage image;
	uint32_t width;
	uint32_t height;
} UploadContext;

void record_texture_upload(VkCommandBuffer cmdbuf, void *data) {
	UploadContext *ctx = (UploadContext *)data;

	VkImageMemoryBarrier barrier1 = {.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
	                                 .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	                                 .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	                                 .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
	                                 .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
	                                 .image = ctx->image,
	                                 .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
	                                 .srcAccessMask = 0,
	                                 .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT};
	vkCmdPipelineBarrier(cmdbuf, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL,
	                     1, &barrier1);

	VkBufferImageCopy region = {.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
	                            .imageExtent = {ctx->width, ctx->height, 1}};
	vkCmdCopyBufferToImage(cmdbuf, ctx->buffer, ctx->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	VkImageMemoryBarrier barrier2 = {.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
	                                 .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	                                 .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	                                 .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
	                                 .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
	                                 .image = ctx->image,
	                                 .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
	                                 .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
	                                 .dstAccessMask = VK_ACCESS_SHADER_READ_BIT};
	vkCmdPipelineBarrier(cmdbuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0,
	                     NULL, 1, &barrier2);
}

int main(const int argc, char **argv) {
	(void)argc;
	(void)argv;
	glfwInit();

	VkInstance inst = create_instance();
	printf("instance created.\n");

	DeviceContext device_ctx = create_device_context(inst);
	WindowContext window_ctx = create_window_and_surface(inst);
	SwapchainContext swap_ctx =
	    create_swapchain(device_ctx.physical, device_ctx.logical, window_ctx.surface, window_ctx.window,
	                     device_ctx.queue_family_index, device_ctx.queue_count);
	VkRenderPass render_pass = create_render_pass(device_ctx.logical, swap_ctx.format);
	PipelineContext pipeline_ctx = create_graphics_pipeline(device_ctx.logical, render_pass, swap_ctx.extent);
	VkFramebuffer *framebuffers = create_framebuffers(device_ctx.logical, render_pass, swap_ctx.image_views,
	                                                  swap_ctx.image_count, swap_ctx.extent);
	CommandContext cmd_ctx =
	    create_command_buffers(device_ctx.logical, device_ctx.queue_family_index, swap_ctx.image_count, render_pass,
	                           framebuffers, pipeline_ctx.pipeline, swap_ctx.extent);

	uint32_t max_frames = 2;
	VkSemaphore *image_available = malloc(max_frames * sizeof(VkSemaphore));
	VkSemaphore *render_finished = malloc(swap_ctx.image_count * sizeof(VkSemaphore));
	VkFence *fences = malloc(max_frames * sizeof(VkFence));
	VkFence *images_in_flight = malloc(swap_ctx.image_count * sizeof(VkFence));

	VkSemaphoreCreateInfo semaphoreInfo = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
	VkFenceCreateInfo fenceInfo = {.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT};

	for(uint32_t i = 0; i < max_frames; i++) {
		vkCreateSemaphore(device_ctx.logical, &semaphoreInfo, NULL, &image_available[i]);
		vkCreateFence(device_ctx.logical, &fenceInfo, NULL, &fences[i]);
	}
	for(uint32_t i = 0; i < swap_ctx.image_count; i++) {
		vkCreateSemaphore(device_ctx.logical, &semaphoreInfo, NULL, &render_finished[i]);
		images_in_flight[i] = VK_NULL_HANDLE;
	}
	printf("semaphores and fences created.\n");

	FontAtlas *atlas = create_font_atlas("SF-Pro-Rounded-Regular.otf", 32);
	if(!atlas) return -1;

	VkBuffer vertexBuffer, stagingBuffer;
	VkDeviceMemory vertexBufferMemory, stagingBufferMemory;
	void *textVboData;

	VkBufferCreateInfo vbInfo = {.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
	                             .size = VBO_SIZE,
	                             .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	                             .sharingMode = VK_SHARING_MODE_EXCLUSIVE};
	vkCreateBuffer(device_ctx.logical, &vbInfo, NULL, &vertexBuffer);
	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(device_ctx.logical, vertexBuffer, &memReqs);
	VkMemoryAllocateInfo allocInfo = {.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
	                                  .allocationSize = memReqs.size,
	                                  .memoryTypeIndex = find_memory_type(device_ctx.physical, memReqs.memoryTypeBits,
	                                                                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
	                                                                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)};
	vkAllocateMemory(device_ctx.logical, &allocInfo, NULL, &vertexBufferMemory);
	vkBindBufferMemory(device_ctx.logical, vertexBuffer, vertexBufferMemory, 0);
	vkMapMemory(device_ctx.logical, vertexBufferMemory, 0, VBO_SIZE, 0, &textVboData);

	VkDeviceSize imgSize = atlas->width * atlas->height;
	VkBufferCreateInfo sbInfo = {.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
	                             .size = imgSize,
	                             .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	                             .sharingMode = VK_SHARING_MODE_EXCLUSIVE};
	vkCreateBuffer(device_ctx.logical, &sbInfo, NULL, &stagingBuffer);
	vkGetBufferMemoryRequirements(device_ctx.logical, stagingBuffer, &memReqs);
	allocInfo.allocationSize = memReqs.size;
	allocInfo.memoryTypeIndex =
	    find_memory_type(device_ctx.physical, memReqs.memoryTypeBits,
	                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	vkAllocateMemory(device_ctx.logical, &allocInfo, NULL, &stagingBufferMemory);
	vkBindBufferMemory(device_ctx.logical, stagingBuffer, stagingBufferMemory, 0);

	void *data;
	vkMapMemory(device_ctx.logical, stagingBufferMemory, 0, imgSize, 0, &data);
	memcpy(data, atlas->pixels, (size_t)imgSize);
	vkUnmapMemory(device_ctx.logical, stagingBufferMemory);

	VkImage fontImage;
	VkImageCreateInfo imgInfo = {.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
	                             .imageType = VK_IMAGE_TYPE_2D,
	                             .format = VK_FORMAT_R8_UNORM,
	                             .extent = {atlas->width, atlas->height, 1},
	                             .mipLevels = 1,
	                             .arrayLayers = 1,
	                             .samples = VK_SAMPLE_COUNT_1_BIT,
	                             .tiling = VK_IMAGE_TILING_OPTIMAL,
	                             .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
	                             .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED};
	vkCreateImage(device_ctx.logical, &imgInfo, NULL, &fontImage);
	vkGetImageMemoryRequirements(device_ctx.logical, fontImage, &memReqs);
	allocInfo.allocationSize = memReqs.size;
	allocInfo.memoryTypeIndex =
	    find_memory_type(device_ctx.physical, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VkDeviceMemory fontMemory;
	vkAllocateMemory(device_ctx.logical, &allocInfo, NULL, &fontMemory);
	vkBindImageMemory(device_ctx.logical, fontImage, fontMemory, 0);

	UploadContext uploadCtx = {stagingBuffer, fontImage, (uint32_t)atlas->width, (uint32_t)atlas->height};
	submit_single_time_command(device_ctx.logical, cmd_ctx.pool, device_ctx.graphics_queue, record_texture_upload,
	                           &uploadCtx);

	vkDestroyBuffer(device_ctx.logical, stagingBuffer, NULL);
	vkFreeMemory(device_ctx.logical, stagingBufferMemory, NULL);

	VkImageView fontView;
	VkImageViewCreateInfo viewInfo = {.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
	                                  .image = fontImage,
	                                  .viewType = VK_IMAGE_VIEW_TYPE_2D,
	                                  .format = VK_FORMAT_R8_UNORM,
	                                  .components = {VK_COMPONENT_SWIZZLE_ONE, VK_COMPONENT_SWIZZLE_ONE,
	                                                 VK_COMPONENT_SWIZZLE_ONE, VK_COMPONENT_SWIZZLE_R},
	                                  .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};
	vkCreateImageView(device_ctx.logical, &viewInfo, NULL, &fontView);
	VkSampler fontSampler;
	VkSamplerCreateInfo sampInfo = {.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
	                                .magFilter = VK_FILTER_LINEAR,
	                                .minFilter = VK_FILTER_LINEAR,
	                                .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
	                                .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE};
	vkCreateSampler(device_ctx.logical, &sampInfo, NULL, &fontSampler);

	VkDescriptorPoolSize poolSize = {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1};
	VkDescriptorPoolCreateInfo poolInfo = {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
	                                       .maxSets = 1,
	                                       .poolSizeCount = 1,
	                                       .pPoolSizes = &poolSize};
	VkDescriptorPool descriptorPool;
	vkCreateDescriptorPool(device_ctx.logical, &poolInfo, NULL, &descriptorPool);
	VkDescriptorSet fontDescriptorSet;
	VkDescriptorSetAllocateInfo allocSet = {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
	                                        .descriptorPool = descriptorPool,
	                                        .descriptorSetCount = 1,
	                                        .pSetLayouts = &pipeline_ctx.descriptor_set_layout};
	vkAllocateDescriptorSets(device_ctx.logical, &allocSet, &fontDescriptorSet);
	VkDescriptorImageInfo descriptorImgInfo = {fontSampler, fontView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
	VkWriteDescriptorSet writeSet = {.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
	                                 .dstSet = fontDescriptorSet,
	                                 .dstBinding = 1,
	                                 .descriptorCount = 1,
	                                 .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
	                                 .pImageInfo = &descriptorImgInfo};
	vkUpdateDescriptorSets(device_ctx.logical, 1, &writeSet, 0, NULL);

	uint32_t cur_frame = 0;
	while(!glfwWindowShouldClose(window_ctx.window)) {
		molten_loop(device_ctx.logical, fences, &cur_frame, swap_ctx.swapchain, image_available, images_in_flight,
		            cmd_ctx.buffers, render_finished, device_ctx.present_queue, device_ctx.graphics_queue, max_frames,
		            atlas, textVboData, render_pass, framebuffers, pipeline_ctx.pipeline, pipeline_ctx.layout,
		            swap_ctx.extent, vertexBuffer, fontDescriptorSet);
	}

	vkDeviceWaitIdle(device_ctx.logical);

	vkFreeCommandBuffers(device_ctx.logical, cmd_ctx.pool, swap_ctx.image_count, cmd_ctx.buffers);
	vkDestroySampler(device_ctx.logical, fontSampler, NULL);
	vkDestroyImageView(device_ctx.logical, fontView, NULL);
	vkDestroyImage(device_ctx.logical, fontImage, NULL);
	vkFreeMemory(device_ctx.logical, fontMemory, NULL);
	vkDestroyDescriptorPool(device_ctx.logical, descriptorPool, NULL);
	vkDestroyDescriptorSetLayout(device_ctx.logical, pipeline_ctx.descriptor_set_layout, NULL);
	vkDestroyBuffer(device_ctx.logical, vertexBuffer, NULL);
	vkFreeMemory(device_ctx.logical, vertexBufferMemory, NULL);
	vkDestroyCommandPool(device_ctx.logical, cmd_ctx.pool, NULL);

	for(uint32_t i = 0; i < max_frames; i++) {
		vkDestroySemaphore(device_ctx.logical, image_available[i], NULL);
		vkDestroyFence(device_ctx.logical, fences[i], NULL);
	}
	for(uint32_t i = 0; i < swap_ctx.image_count; i++) {
		vkDestroySemaphore(device_ctx.logical, render_finished[i], NULL);
		vkDestroyFramebuffer(device_ctx.logical, framebuffers[i], NULL);
		vkDestroyImageView(device_ctx.logical, swap_ctx.image_views[i], NULL);
	}

	vkDestroyPipeline(device_ctx.logical, pipeline_ctx.pipeline, NULL);
	vkDestroyPipelineLayout(device_ctx.logical, pipeline_ctx.layout, NULL);
	vkDestroyRenderPass(device_ctx.logical, render_pass, NULL);
	vkDestroySwapchainKHR(device_ctx.logical, swap_ctx.swapchain, NULL);
	vkDestroySurfaceKHR(inst, window_ctx.surface, NULL);
	glfwDestroyWindow(window_ctx.window);
	vkDestroyDevice(device_ctx.logical, NULL);
	vkDestroyInstance(inst, NULL);

	free(image_available);
	free(render_finished);
	free(fences);
	free(images_in_flight);
	free(framebuffers);
	free(cmd_ctx.buffers);
	free(swap_ctx.images);
	free(swap_ctx.image_views);
	free(atlas->pixels);
	free(atlas);
	glfwTerminate();
	return 0;
}
