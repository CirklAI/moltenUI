#ifndef SYNC_H
#define SYNC_H

#include <vulkan/vulkan.h>

typedef struct {
	VkSemaphore *image_available;
	VkSemaphore *render_finished;
	VkFence *fences;
	uint32_t max_frames;
} SyncContext;

SyncContext create_sync_objects(VkDevice dev, uint32_t max_frames);

#endif
