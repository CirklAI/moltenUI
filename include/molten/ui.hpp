#pragma once
#include <glm/glm.hpp>
#include <map>
#include <string>

namespace UI {
struct InputState {
	float scrollOffset = 0.0f;
	float targetScrollOffset = 0.0f;
};

struct State {
	int focusedId = -1;
	std::map<int, InputState> inputStates;
};
inline State state;

void update_input_states();

bool Button(int id, glm::vec2 pos, glm::vec2 size, const std::string &label);
void InputField(int id, std::string &text, glm::vec2 pos, glm::vec2 size);
void Text(const std::string &label, glm::vec2 pos, glm::vec4 color = glm::vec4(1.0f));
}; // namespace UI
