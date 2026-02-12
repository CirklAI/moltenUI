#include "renderpass.h"
#include <stdio.h>

VkRenderPass create_render_pass(VkDevice dev, VkFormat format) {
	VkAttachmentDescription attach_descp;
	attach_descp.flags = 0;
	attach_descp.format = format;
	attach_descp.samples = VK_SAMPLE_COUNT_1_BIT;
	attach_descp.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attach_descp.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attach_descp.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attach_descp.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attach_descp.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attach_descp.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	printf("attachment description filled.\n");

	VkAttachmentReference attach_ref;
	attach_ref.attachment = 0;
	attach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	printf("attachment reference filled.\n");

	VkSubpassDescription subp_descp;
	subp_descp.flags = 0;
	subp_descp.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subp_descp.inputAttachmentCount = 0;
	subp_descp.pInputAttachments = NULL;
	subp_descp.colorAttachmentCount = 1;
	subp_descp.pColorAttachments = &attach_ref;
	subp_descp.pResolveAttachments = NULL;
	subp_descp.pDepthStencilAttachment = NULL;
	subp_descp.preserveAttachmentCount = 0;
	subp_descp.pPreserveAttachments = NULL;
	printf("subpass description filled.\n");

	VkSubpassDependency subp_dep;
	subp_dep.srcSubpass = VK_SUBPASS_EXTERNAL;
	subp_dep.dstSubpass = 0;
	subp_dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subp_dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subp_dep.srcAccessMask = 0;
	subp_dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subp_dep.dependencyFlags = 0;
	printf("subpass dependency created.\n");

	VkRenderPassCreateInfo rendp_cre_info;
	rendp_cre_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rendp_cre_info.pNext = NULL;
	rendp_cre_info.flags = 0;
	rendp_cre_info.attachmentCount = 1;
	rendp_cre_info.pAttachments = &attach_descp;
	rendp_cre_info.subpassCount = 1;
	rendp_cre_info.pSubpasses = &subp_descp;
	rendp_cre_info.dependencyCount = 1;
	rendp_cre_info.pDependencies = &subp_dep;

	VkRenderPass rendp;
	vkCreateRenderPass(dev, &rendp_cre_info, NULL, &rendp);
	printf("render pass created.\n");

	return rendp;
}
