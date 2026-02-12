#include "ui.hpp"
#include "components/input.hpp"
#include "init.hpp"
#include "render.hpp"
#include "utils/render.hpp"

namespace UI {
void Text(const std::string &label, glm::vec2 pos, glm::vec4 color) {
	Render::fontRenderer->draw_text(Init::commandBuffers[Init::currentFrame], label, pos, color);
}

bool Button(int id, glm::vec2 pos, glm::vec2 size, const std::string &label) {
	bool hovered = Input::is_hovered(pos, size);
	bool pressed = hovered && Input::is_mouse_down(GLFW_MOUSE_BUTTON_LEFT);

	glm::vec4 bg = pressed ? glm::vec4(0.1f, 0.1f, 0.12f, 1.0f)
	                       : (hovered ? glm::vec4(0.25f, 0.25f, 0.28f, 1.0f) : glm::vec4(0.15f, 0.15f, 0.18f, 1.0f));

	RenderUtils::draw_rect(pos, size, bg);
	Text(label, {pos.x + 10, pos.y + size.y * 0.7f}, glm::vec4(1.0f));

	return hovered && Input::is_mouse_pressed(GLFW_MOUSE_BUTTON_LEFT);
}

void InputField(int id, std::string &text, glm::vec2 pos, glm::vec2 size) {
	bool hovered = Input::is_hovered(pos, size);
	if(Input::is_mouse_pressed(GLFW_MOUSE_BUTTON_LEFT)) state.focusedId = hovered ? id : -1;

	if(state.focusedId == id) {
		text += Input::get_chars();
		if(Input::is_key_pressed(GLFW_KEY_BACKSPACE) && !text.empty()) text.pop_back();
	}

	glm::vec4 border = (state.focusedId == id) ? glm::vec4(0.3f, 0.5f, 1.0f, 1.0f) : glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
	RenderUtils::draw_rect(pos - glm::vec2(1), size + glm::vec2(2), border);
	RenderUtils::draw_rect(pos, size, glm::vec4(0.05f, 0.05f, 0.05f, 1.0f));
	Text(text, {pos.x + 10, pos.y + size.y * 0.7f}, glm::vec4(0.9f));
}
} // namespace UI
