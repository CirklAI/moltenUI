#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>

namespace Input {
inline glm::vec2 mousePos;
inline glm::vec2 contentScale = {1.0f, 1.0f};
inline bool mouseButtons[GLFW_MOUSE_BUTTON_LAST];
inline bool mouseButtonsLast[GLFW_MOUSE_BUTTON_LAST];
inline bool keys[GLFW_KEY_LAST];
inline bool keysLast[GLFW_KEY_LAST];
inline std::string charBuffer;

void init(GLFWwindow *window);
void update(GLFWwindow *window);

glm::vec2 get_mouse_pos();
glm::vec2 get_content_scale();

bool is_key_down(int key);
bool is_key_pressed(int key);
bool is_alt_down();
bool is_mouse_down(int button);
bool is_mouse_pressed(int button);
bool is_hovered(glm::vec2 pos, glm::vec2 size);
std::string get_chars();
} // namespace Input
