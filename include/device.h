#ifndef DEVICE_H
#define DEVICE_H

#include <vulkan/vulkan.h>

typedef struct {
	VkPhysicalDevice physical;
	VkDevice logical;
	uint32_t queue_family_index;
	VkQueue graphics_queue;
	VkQueue present_queue;
	uint32_t queue_count;
} DeviceContext;

DeviceContext create_device_context(VkInstance inst);

#endif
