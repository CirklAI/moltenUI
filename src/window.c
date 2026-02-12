#include "window.h"
#include <stdio.h>

WindowContext create_window_and_surface(VkInstance inst) {
	WindowContext ctx;

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	ctx.window = glfwCreateWindow(600, 600, "", NULL, NULL);
	printf("window created.\n");

	glfwCreateWindowSurface(inst, ctx.window, NULL, &ctx.surface);
	printf("surface created.\n");

	return ctx;
}
