#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <vulkan/vulkan.h>

VkFramebuffer *create_framebuffers(VkDevice dev, VkRenderPass render_pass, VkImageView *image_views,
                                   uint32_t image_count, VkExtent2D extent);

#endif
