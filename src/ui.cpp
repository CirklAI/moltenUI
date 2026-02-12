#include "ui.hpp"
#include "components/input.hpp"
#include "init.hpp"
#include "render.hpp"
#include "utils/render.hpp"
#include <algorithm>
#include <vector>

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

	if(hovered) {
		glm::vec4 accent = glm::vec4(0.3f, 0.6f, 1.0f, 1.0f);
		RenderUtils::draw_rect(pos + glm::vec2(0, size.y - 2), {size.x, 2}, accent);
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
	RenderUtils::draw_rect(pos - glm::vec2(1), size + glm::vec2(2), border);
	RenderUtils::draw_rect(pos, size, glm::vec4(0.05f, 0.05f, 0.05f, 1.0f));

	float charWidth = 12.0f;
	float textStart = pos.x + 10;
	float maxWidth = size.x - 20;

	std::string displayText = text.empty() ? "Type here..." : text;
	glm::vec4 textColor = text.empty() ? glm::vec4(0.4f) : glm::vec4(0.9f);

	float lineHeight = 18.0f;
	int maxLines = (int)(size.y / lineHeight);
	maxLines = std::max(1, maxLines);

	std::vector<std::string> lines;
	std::string line;
	for(size_t i = 0; i < displayText.size(); i++) {
		char c = displayText[i];
		if(c == '\n' || (line.length() > 0 && line.length() * charWidth > maxWidth && c == ' ')) {
			lines.push_back(line);
			line.clear();
		} else {
			line += c;
		}
	}
	if(!line.empty() || displayText == "Type here...") lines.push_back(line);

	while((int)lines.size() > maxLines) {
		lines.erase(lines.begin());
	}

	float y = pos.y + (size.y - lines.size() * lineHeight) / 2 + lineHeight * 0.7f;
	for(const auto &l : lines) {
		Text(l, {textStart, y}, textColor);
		y += lineHeight;
	}

	if(focused) {
		std::string visibleText = lines.empty() ? "" : lines.back();
		float cursorX = textStart + visibleText.length() * charWidth;
		RenderUtils::draw_rect({cursorX, pos.y + (size.y - lines.size() * lineHeight) / 2 + 5}, {2, lineHeight},
		                       glm::vec4(1.0f));
	}
}
} // namespace UI
