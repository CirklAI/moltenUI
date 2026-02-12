#pragma once
// clang-format off
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
// clang-format on
#include "VkBootstrap.h"
#include "vk_mem_alloc.h"
#include <vector>

constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

namespace Init {

extern vkb::Instance vkbInstance;
extern vkb::PhysicalDevice vkbPhysicalDevice;
extern vkb::Device vkbDevice;
extern vkb::Swapchain vkbSwapchain;

extern VkInstance instance;
extern VkPhysicalDevice physicalDevice;
extern VkDevice device;
extern VkSurfaceKHR surface;
extern VkQueue graphicsQueue;
extern VkQueue presentQueue;
extern VkRenderPass renderPass;
extern VkPipelineLayout pipelineLayout;
extern VkPipeline graphicsPipeline;

extern VmaAllocator allocator;

extern std::vector<VkImage> swapchainImages;
extern std::vector<VkImageView> swapchainImageViews;
extern std::vector<VkFramebuffer> framebuffers;
extern std::vector<VkCommandBuffer> commandBuffers;
extern VkCommandPool commandPool;

extern std::vector<VkSemaphore> imageAvailableSemaphores;
extern std::vector<VkSemaphore> renderFinishedSemaphores;
extern std::vector<VkFence> inFlightFences;
extern size_t currentFrame;

void init_vulkan(const char *title, GLFWwindow *window);
void cleanup_vulkan();
} // namespace Init
