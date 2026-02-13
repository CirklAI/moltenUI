#include "components/input.hpp"

namespace Input {
void init(GLFWwindow *window) {
	float xscale, yscale;
	glfwGetWindowContentScale(window, &xscale, &yscale);
	contentScale = {xscale, yscale};

	glfwSetCursorPosCallback(window, [](GLFWwindow *, double x, double y) { mousePos = {(float)x, (float)y}; });

	glfwSetMouseButtonCallback(window, [](GLFWwindow *, int button, int action, int mods) {
		if(button < GLFW_MOUSE_BUTTON_LAST) mouseButtons[button] = (action == GLFW_PRESS);
	});

	glfwSetKeyCallback(window, [](GLFWwindow *, int key, int scancode, int action, int mods) {
		if(key >= 0 && key < GLFW_KEY_LAST) keys[key] = (action != GLFW_RELEASE);
	});

	glfwSetCharCallback(window, [](GLFWwindow *, unsigned int codepoint) { charBuffer += (char)codepoint; });
}

void update(GLFWwindow *window) {
	float xscale, yscale;
	glfwGetWindowContentScale(window, &xscale, &yscale);
	contentScale = {xscale, yscale};

	for(int i = 0; i < GLFW_MOUSE_BUTTON_LAST; i++)
		mouseButtonsLast[i] = mouseButtons[i];
	for(int i = 0; i < GLFW_KEY_LAST; i++)
		keysLast[i] = keys[i];
	charBuffer.clear();
}

glm::vec2 get_mouse_pos() {
	return mousePos;
}

glm::vec2 get_content_scale() {
	return contentScale;
}

bool is_key_down(int key) {
	return keys[key];
}
bool is_key_pressed(int key) {
	return keys[key] && !keysLast[key];
}

bool is_alt_down() {
	return keys[GLFW_KEY_LEFT_ALT] || keys[GLFW_KEY_RIGHT_ALT];
}

bool is_mouse_down(int button) {
	return mouseButtons[button];
}
bool is_mouse_pressed(int button) {
	return mouseButtons[button] && !mouseButtonsLast[button];
}

bool is_hovered(glm::vec2 pos, glm::vec2 size) {
	glm::vec2 m = get_mouse_pos();
	return m.x >= pos.x && m.x <= pos.x + size.x && m.y >= pos.y && m.y <= pos.y + size.y;
}

std::string get_chars() {
	return charBuffer;
}
} // namespace Input
