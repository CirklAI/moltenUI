#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

typedef struct {
	VkSwapchainKHR swapchain;
	VkImage *images;
	VkImageView *image_views;
	uint32_t image_count;
	VkFormat format;
	VkExtent2D extent;
} SwapchainContext;

SwapchainContext create_swapchain(VkPhysicalDevice phy_dev, VkDevice dev, VkSurfaceKHR surface, GLFWwindow *window,
                                  uint32_t queue_family_index, uint32_t queue_count);

#endif
