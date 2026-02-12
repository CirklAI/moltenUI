// clang-format off
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
// clang-format on
#include "components/input.hpp"
#include "init.hpp"
#include "render.hpp"
#include "vk_mem_alloc.h"
#include <iostream>

#define TITLE "Balls"
int main() {
	if(!glfwInit()) {
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return -1;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow *window = glfwCreateWindow(800, 600, TITLE, NULL, NULL);
	if(window == NULL) {
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	Input::init(window);
	Init::init_vulkan(TITLE, window);
	while(!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		Render::draw_frame();
	}

	Init::cleanup_vulkan();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
