#include "framebuffer.h"
#include <stdio.h>
#include <stdlib.h>

VkFramebuffer *create_framebuffers(VkDevice dev, VkRenderPass render_pass, VkImageView *image_views,
                                   uint32_t image_count, VkExtent2D extent) {
	VkFramebuffer *frame_buffs = malloc(image_count * sizeof(VkFramebuffer));

	for(uint32_t i = 0; i < image_count; i++) {
		VkFramebufferCreateInfo frame_buff_cre_info;
		frame_buff_cre_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frame_buff_cre_info.pNext = NULL;
		frame_buff_cre_info.flags = 0;
		frame_buff_cre_info.renderPass = render_pass;
		frame_buff_cre_info.attachmentCount = 1;
		frame_buff_cre_info.pAttachments = &image_views[i];
		frame_buff_cre_info.width = extent.width;
		frame_buff_cre_info.height = extent.height;
		frame_buff_cre_info.layers = 1;

		vkCreateFramebuffer(dev, &frame_buff_cre_info, NULL, &frame_buffs[i]);
		printf("framebuffer %d created.\n", i);
	}

	return frame_buffs;
}
