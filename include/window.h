#ifndef WINDOW_H
#define WINDOW_H

// clang-format off
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
// clang-format on

typedef struct {
	GLFWwindow *window;
	VkSurfaceKHR surface;
} WindowContext;

WindowContext create_window_and_surface(VkInstance inst);

#endif
