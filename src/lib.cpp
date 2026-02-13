#include "molten/molten.hpp"
// clang-format off
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
// clang-format on
#include "components/input.hpp"
#include "init.hpp"
#include "render.hpp"
#include "vk_mem_alloc.h"
#include <iostream>

Molten::Molten(const std::string name, const unsigned int width, const unsigned int height)
    : name(name), width(width), height(height), window(nullptr) {}

Molten::~Molten() {
	Init::cleanup_vulkan();
	glfwDestroyWindow(window);
	glfwTerminate();
}

int Molten::init() {
	if(!glfwInit()) {
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return -1;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	window = glfwCreateWindow(this->width, this->height, this->name.c_str(), NULL, NULL);

	if(window == NULL) {
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	Input::init(window);
	Init::init_vulkan(name.c_str(), window);

	return 0;
}

int Molten::run(std::function<void()> callback) const {
	while(!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		Render::draw_frame(callback);
		Input::update(window);
	}

	return 0;
}
