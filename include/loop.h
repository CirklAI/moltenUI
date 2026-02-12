#ifndef LOOP_H
#define LOOP_H

#include "font.h"
#include <vulkan/vulkan.h>

void molten_loop(VkDevice dev, VkFence *fens, uint32_t *cur_frame, VkSwapchainKHR swap, VkSemaphore *semps_img_avl,
                 VkFence *fens_img, VkCommandBuffer *cmd_buffers, VkSemaphore *semps_rend_fin, VkQueue q_pres,
                 VkQueue q_graph, uint32_t max_frames, FontAtlas *atlas, void *mapped_vbo_data,
                 VkRenderPass render_pass, VkFramebuffer *framebuffers, VkPipeline pipeline, VkPipelineLayout layout,
                 VkExtent2D extent, VkBuffer vertex_buffer, VkDescriptorSet descriptor_set);

#endif
