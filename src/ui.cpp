#include "molten/ui.hpp"
#include "components/input.hpp"
#include "font.hpp"
#include "init.hpp"
#include "render.hpp"
#include <algorithm>
#include <vector>

namespace UI {
void Text(const std::string &label, glm::vec2 pos, glm::vec4 color) {
	VkCommandBuffer cmd = Init::commandBuffers[Init::currentFrame];
	Render::fontRenderer->draw_text(cmd, label, pos, color);
}

bool Button(int id, glm::vec2 pos, glm::vec2 size, const std::string &label) {
	bool hovered = Input::is_hovered(pos, size);
	bool pressed = hovered && Input::is_mouse_down(GLFW_MOUSE_BUTTON_LEFT);
	glm::vec4 bg = pressed ? glm::vec4(0.1f, 0.1f, 0.12f, 1.0f)
	                       : (hovered ? glm::vec4(0.25f, 0.25f, 0.28f, 1.0f) : glm::vec4(0.15f, 0.15f, 0.18f, 1.0f));

	Render::draw_rect(pos.x, pos.y, size.x, size.y, bg);

	if(hovered) {
		glm::vec4 accent = glm::vec4(0.3f, 0.6f, 1.0f, 1.0f);
		Render::draw_rect(pos.x, pos.y + size.y - 2, size.x, 2, accent);
	}

	Text(label, {pos.x + 10, pos.y + size.y * 0.7f}, glm::vec4(1.0f));
	return hovered && Input::is_mouse_pressed(GLFW_MOUSE_BUTTON_LEFT);
}

void InputField(int id, std::string &text, glm::vec2 pos, glm::vec2 size) {
	bool hovered = Input::is_hovered(pos, size);
	if(Input::is_mouse_pressed(GLFW_MOUSE_BUTTON_LEFT)) {
		state.focusedId = hovered ? id : -1;
	}

	if(state.focusedId == id) {
		std::string chars = Input::get_chars();
		text += chars;

		if(Input::is_key_pressed(GLFW_KEY_BACKSPACE)) {
			if(Input::is_alt_down() && !text.empty()) {
				size_t p = text.find_last_of(" \t", text.length() - 2);
				if(p == std::string::npos) p = 0;
				else p++;
				text.erase(p);
			} else if(!text.empty()) {
				text.pop_back();
			}
		}

		if(Input::is_key_pressed(GLFW_KEY_DELETE) && !text.empty()) {
			if(Input::is_alt_down()) {
				size_t p = text.find(' ');
				if(p != std::string::npos) text.erase(0, p + 1);
				else text.clear();
			} else {
				text.erase(text.begin());
			}
		}
	}

	bool focused = state.focusedId == id;
	glm::vec4 border = focused ? glm::vec4(0.3f, 0.5f, 1.0f, 1.0f)
	                           : (hovered ? glm::vec4(0.4f, 0.4f, 0.45f, 1.0f) : glm::vec4(0.2f, 0.2f, 0.2f, 1.0f));

	Render::draw_rect(pos.x - 1, pos.y - 1, size.x + 2, size.y + 2, border);
	Render::draw_rect(pos.x, pos.y, size.x, size.y, {0.05f, 0.05f, 0.05f, 1.0f});

	float textStart = pos.x + 10;
	float maxWidth = size.x - 20;
	std::string displayText = text.empty() ? "Type here..." : text;
	glm::vec4 textColor = text.empty() ? glm::vec4(0.4f) : glm::vec4(0.9f);
	float lineHeight = Render::fontRenderer->get_line_height();
	float textY = pos.y + (size.y - lineHeight) / 2.0f + lineHeight * 0.75f;

	float textWidth = Render::fontRenderer->measure_text(displayText);
	float &scrollX = state.inputStates[id].scrollOffset;
	float cursorX = Render::fontRenderer->measure_text(displayText);
	float visibleWidth = maxWidth;
	float targetScrollX = 0.0f;

	if(textWidth > visibleWidth) {
		targetScrollX = cursorX - visibleWidth + 5;
		targetScrollX = std::max(0.0f, std::min(targetScrollX, textWidth - visibleWidth));
	}

	scrollX += (targetScrollX - scrollX) * 0.3f;
	if(std::abs(targetScrollX - scrollX) < 0.5f) {
		scrollX = targetScrollX;
	}

	Render::set_scissor(textStart, pos.y, maxWidth, size.y);

	float drawX = textStart - scrollX;
	Text(displayText, {drawX, textY}, textColor);

	if(focused) {
		float cursorDrawX = textStart + cursorX - scrollX;
		if(cursorDrawX >= textStart && cursorDrawX <= textStart + visibleWidth) {
			float cursorY = pos.y + (size.y - lineHeight) / 2.0f;
			Render::draw_rect(cursorDrawX, cursorY, 2, lineHeight, glm::vec4(1.0f));
		}
	}

	Render::reset_scissor();
}

} // namespace UI
