#include "swapchain.h"
#include <stdio.h>
#include <stdlib.h>

SwapchainContext create_swapchain(VkPhysicalDevice phy_dev, VkDevice dev, VkSurfaceKHR surface, GLFWwindow *window,
                                  uint32_t queue_family_index, uint32_t queue_count) {
	SwapchainContext ctx;

	VkBool32 phy_surf_supported;
	vkGetPhysicalDeviceSurfaceSupportKHR(phy_dev, queue_family_index, surface, &phy_surf_supported);
	if(phy_surf_supported == VK_TRUE) {
		printf("surface supported.\n");
	} else {
		printf("warning:surface unsupported!\n");
	}

	VkSurfaceCapabilitiesKHR surf_caps;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phy_dev, surface, &surf_caps);
	printf("fetched caps from surface.\n");

	char extent_suitable = 1;
	int wind_w, wind_h;
	glfwGetFramebufferSize(window, &wind_w, &wind_h);
	VkExtent2D actual_extent;
	actual_extent.width = wind_w;
	actual_extent.height = wind_h;
	if(surf_caps.currentExtent.width != (uint32_t)wind_w || surf_caps.currentExtent.height != (uint32_t)wind_h) {
		extent_suitable = 0;
		printf("actual extent size doesn't match framebuffers, resizing...\n");
		actual_extent.width =
		    (uint32_t)wind_w > surf_caps.maxImageExtent.width ? surf_caps.maxImageExtent.width : wind_w;
		actual_extent.width =
		    (uint32_t)wind_w < surf_caps.minImageExtent.width ? surf_caps.minImageExtent.width : wind_w;
		actual_extent.height =
		    (uint32_t)wind_h > surf_caps.maxImageExtent.height ? surf_caps.maxImageExtent.height : wind_h;
		actual_extent.height =
		    (uint32_t)wind_h < surf_caps.minImageExtent.height ? surf_caps.minImageExtent.height : wind_h;
	}

	uint32_t surf_form_count;
	vkGetPhysicalDeviceSurfaceFormatsKHR(phy_dev, surface, &surf_form_count, NULL);
	VkSurfaceFormatKHR surf_forms[surf_form_count];
	vkGetPhysicalDeviceSurfaceFormatsKHR(phy_dev, surface, &surf_form_count, surf_forms);
	printf("fetched %d surface formats.\n", surf_form_count);
	for(uint32_t i = 0; i < surf_form_count; i++) {
		printf("format:%d\tcolorspace:%d\n", surf_forms[i].format, surf_forms[i].colorSpace);
	}

	uint32_t pres_mode_count;
	vkGetPhysicalDeviceSurfacePresentModesKHR(phy_dev, surface, &pres_mode_count, NULL);
	VkPresentModeKHR pres_modes[pres_mode_count];
	vkGetPhysicalDeviceSurfacePresentModesKHR(phy_dev, surface, &pres_mode_count, pres_modes);
	printf("fetched %d present modes.\n", pres_mode_count);
	char mailbox_mode_supported = 0;
	for(uint32_t i = 0; i < pres_mode_count; i++) {
		printf("present mode:%d\n", pres_modes[i]);
		if(pres_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
			printf("mailbox present mode supported.\n");
			mailbox_mode_supported = 1;
		}
	}

	VkSwapchainCreateInfoKHR swap_cre_info;
	swap_cre_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swap_cre_info.pNext = NULL;
	swap_cre_info.flags = 0;
	swap_cre_info.surface = surface;
	swap_cre_info.minImageCount = surf_caps.minImageCount + 1;
	swap_cre_info.imageFormat = surf_forms[0].format;
	swap_cre_info.imageColorSpace = surf_forms[0].colorSpace;
	swap_cre_info.imageExtent = extent_suitable ? surf_caps.currentExtent : actual_extent;
	swap_cre_info.imageArrayLayers = 1;
	swap_cre_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swap_cre_info.imageSharingMode = (queue_count == 1) ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;
	swap_cre_info.queueFamilyIndexCount = (queue_count == 1) ? 0 : 2;
	uint32_t qf_indices[2] = {0, 1};
	swap_cre_info.pQueueFamilyIndices = (queue_count == 1) ? NULL : qf_indices;
	swap_cre_info.preTransform = surf_caps.currentTransform;
	swap_cre_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swap_cre_info.presentMode = mailbox_mode_supported ? VK_PRESENT_MODE_MAILBOX_KHR : VK_PRESENT_MODE_FIFO_KHR;
	swap_cre_info.clipped = VK_TRUE;
	swap_cre_info.oldSwapchain = VK_NULL_HANDLE;

	vkCreateSwapchainKHR(dev, &swap_cre_info, NULL, &ctx.swapchain);
	printf("swapchain created.\n");

	ctx.format = surf_forms[0].format;
	ctx.extent = swap_cre_info.imageExtent;

	vkGetSwapchainImagesKHR(dev, ctx.swapchain, &ctx.image_count, NULL);
	ctx.images = malloc(ctx.image_count * sizeof(VkImage));
	vkGetSwapchainImagesKHR(dev, ctx.swapchain, &ctx.image_count, ctx.images);
	printf("%d images fetched from swapchain.\n", ctx.image_count);

	ctx.image_views = malloc(ctx.image_count * sizeof(VkImageView));
	VkComponentMapping image_view_rgba_comp;
	image_view_rgba_comp.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_rgba_comp.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_rgba_comp.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_rgba_comp.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	VkImageSubresourceRange image_view_subres;
	image_view_subres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	image_view_subres.baseMipLevel = 0;
	image_view_subres.levelCount = 1;
	image_view_subres.baseArrayLayer = 0;
	image_view_subres.layerCount = swap_cre_info.imageArrayLayers;

	for(uint32_t i = 0; i < ctx.image_count; i++) {
		VkImageViewCreateInfo image_view_cre_info;
		image_view_cre_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_cre_info.pNext = NULL;
		image_view_cre_info.flags = 0;
		image_view_cre_info.image = ctx.images[i];
		image_view_cre_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		image_view_cre_info.format = ctx.format;
		image_view_cre_info.components = image_view_rgba_comp;
		image_view_cre_info.subresourceRange = image_view_subres;
		vkCreateImageView(dev, &image_view_cre_info, NULL, &ctx.image_views[i]);
		printf("image view %d created.\n", i);
	}

	return ctx;
}
