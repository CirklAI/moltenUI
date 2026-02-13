#pragma once
#include <glm/glm.hpp>
#include <string>

namespace UI {
struct State {
	int focusedId = -1;
};
inline State state;

bool Button(int id, glm::vec2 pos, glm::vec2 size, const std::string &label);
void InputField(int id, std::string &text, glm::vec2 pos, glm::vec2 size);
void Text(const std::string &label, glm::vec2 pos, glm::vec4 color = glm::vec4(1.0f));
void update();
}; // namespace UI
