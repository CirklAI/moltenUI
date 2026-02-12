#include "sync.h"
#include <stdio.h>
#include <stdlib.h>

SyncContext create_sync_objects(VkDevice dev, uint32_t max_frames) {
	SyncContext ctx;
	ctx.max_frames = max_frames;

	ctx.image_available = malloc(max_frames * sizeof(VkSemaphore));
	ctx.render_finished = malloc(max_frames * sizeof(VkSemaphore));
	ctx.fences = malloc(max_frames * sizeof(VkFence));

	VkSemaphoreCreateInfo semp_cre_info;
	semp_cre_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semp_cre_info.pNext = NULL;
	semp_cre_info.flags = 0;

	VkFenceCreateInfo fen_cre_info;
	fen_cre_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fen_cre_info.pNext = NULL;
	fen_cre_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for(uint32_t i = 0; i < max_frames; i++) {
		vkCreateSemaphore(dev, &semp_cre_info, NULL, &ctx.image_available[i]);
		vkCreateSemaphore(dev, &semp_cre_info, NULL, &ctx.render_finished[i]);
		vkCreateFence(dev, &fen_cre_info, NULL, &ctx.fences[i]);
	}
	printf("semaphores and fences created.\n");

	return ctx;
}
