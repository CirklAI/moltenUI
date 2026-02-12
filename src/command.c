#include "command.h"
#include <stdio.h>
#include <stdlib.h>

CommandContext create_command_buffers(VkDevice device, uint32_t queue_family_index, uint32_t count,
                                      VkRenderPass render_pass, VkFramebuffer *framebuffers, VkPipeline pipeline,
                                      VkExtent2D extent) {
	(void)render_pass;
	(void)framebuffers;
	(void)pipeline;
	(void)extent;

	CommandContext ctx;

	VkCommandPoolCreateInfo poolInfo = {.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
	                                    .queueFamilyIndex = queue_family_index,
	                                    .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT};

	if(vkCreateCommandPool(device, &poolInfo, NULL, &ctx.pool) != VK_SUCCESS) {
		printf("failed to create command pool!\n");
		exit(1);
	}

	ctx.buffers = malloc(sizeof(VkCommandBuffer) * count);

	VkCommandBufferAllocateInfo allocInfo = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
	                                         .commandPool = ctx.pool,
	                                         .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
	                                         .commandBufferCount = count};

	if(vkAllocateCommandBuffers(device, &allocInfo, ctx.buffers) != VK_SUCCESS) {
		printf("failed to allocate command buffers!\n");
		exit(1);
	}

	printf("command pool created and buffers allocated (empty).\n");

	return ctx;
}
