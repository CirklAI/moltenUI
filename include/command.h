#ifndef COMMAND_H
#define COMMAND_H

#include <vulkan/vulkan.h>

typedef struct {
	VkCommandPool pool;
	VkCommandBuffer *buffers;
} CommandContext;

CommandContext create_command_buffers(VkDevice dev, uint32_t queue_family_index, uint32_t buffer_count,
                                      VkRenderPass render_pass, VkFramebuffer *framebuffers, VkPipeline pipeline,
                                      VkExtent2D extent);

#endif
