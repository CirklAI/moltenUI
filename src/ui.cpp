#include "molten/ui.hpp"
#include "components/input.hpp"
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

	Render::draw_rect(pos.x - 1, pos.y - 1, size.x, size.y + 2, border);
	Render::draw_rect(pos.x, pos.y, size.x, size.y, {0.05f, 0.05f, 0.05f, 1.0f});

	float charWidth = 8.0f;
	float textStart = pos.x + 10;
	float maxWidth = size.x - 20;
	std::string displayText = text.empty() ? "Type here..." : text;
	glm::vec4 textColor = text.empty() ? glm::vec4(0.4f) : glm::vec4(0.9f);
	float lineHeight = 20.0f;
	int maxLines = std::max(1, (int)((size.y - 10) / lineHeight));

	std::vector<std::string> lines;
	std::string line;
	float currentLineWidth = 0;

	for(size_t i = 0; i < displayText.size(); i++) {
		char c = displayText[i];
		float charW = (c == ' ') ? charWidth * 0.5f : charWidth;

		if(c == '\n') {
			lines.push_back(line);
			line.clear();
			currentLineWidth = 0;
		} else if(currentLineWidth + charW > maxWidth && !line.empty()) {
			size_t lastSpace = line.find_last_of(' ');
			if(lastSpace != std::string::npos && lastSpace > 0 && i - line.length() + lastSpace < displayText.size()) {
				lines.push_back(line.substr(0, lastSpace));
				line = line.substr(lastSpace + 1) + c;
				currentLineWidth = line.length() * charWidth;
			} else {
				lines.push_back(line);
				line = std::string(1, c);
				currentLineWidth = charW;
			}
		} else {
			line += c;
			currentLineWidth += charW;
		}
	}
	if(!line.empty() || displayText == "Type here...") lines.push_back(line);

	int scrollOffset = 0;
	if((int)lines.size() > maxLines) {
		scrollOffset = (int)lines.size() - maxLines;
	}

	float y = pos.y + 10;
	for(int i = scrollOffset; i < std::min((int)lines.size(), scrollOffset + maxLines); i++) {
		Text(lines[i], {textStart, y + lineHeight * 0.6f}, textColor);
		y += lineHeight;
	}

	if(focused && !lines.empty()) {
		int cursorLine = (int)lines.size() - 1 - scrollOffset;
		if(cursorLine >= 0 && cursorLine < maxLines) {
			const std::string &lastLine = lines.back();
			float cursorX = textStart + lastLine.length() * charWidth;
			float cursorY = pos.y + 10 + cursorLine * lineHeight + 2;
			Render::draw_rect(cursorX, cursorY, 2, lineHeight - 6, glm::vec4(1.0f));
		}
	}
}

} // namespace UI
