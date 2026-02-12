#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace Input {
inline glm::vec2 mousePos;
inline bool mouseButtons[GLFW_MOUSE_BUTTON_LAST];
inline bool mouseButtonsLast[GLFW_MOUSE_BUTTON_LAST];
inline bool keys[GLFW_KEY_LAST];
inline bool keysLast[GLFW_KEY_LAST];
inline std::string charBuffer;

void init(GLFWwindow *window);
void update();

bool is_key_down(int key);
bool is_key_pressed(int key);
bool is_alt_down();
bool is_mouse_down(int button);
bool is_mouse_pressed(int button);
bool is_hovered(glm::vec2 pos, glm::vec2 size);
std::string get_chars();
} // namespace Input
