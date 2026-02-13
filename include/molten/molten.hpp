#pragma once

#include "molten/theme.hpp"
#include "molten/ui.hpp"
#include <functional>
#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Molten final {
  private:
	std::string name;
	unsigned int width;
	unsigned int height;
	GLFWwindow *window;

  public:
	Molten(const std::string name, const unsigned int width, const unsigned int height);
	~Molten();
	Molten(const Molten &) = delete;
	Molten &operator=(const Molten &) = delete;

	int init();
	int run(std::function<void()> callback) const;
};
