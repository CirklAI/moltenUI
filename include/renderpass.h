#ifndef RENDERPASS_H
#define RENDERPASS_H

#include <vulkan/vulkan.h>

VkRenderPass create_render_pass(VkDevice dev, VkFormat format);

#endif
