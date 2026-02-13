#include "molten/ui.hpp"
#include "components/input.hpp"
#include "font.hpp"
#include "init.hpp"
#include "render.hpp"
#include <algorithm>
#include <cmath>

namespace UI {

void draw_text(const std::string &label, glm::vec2 pos, glm::vec4 color) {
	VkCommandBuffer cmd = Init::commandBuffers[Init::currentFrame];
	Render::fontRenderer->draw_text(cmd, label, pos, color);
}

bool button(int id, glm::vec2 pos, glm::vec2 size, const std::string &label) {
	auto &theme = Theme::get_current();
	bool hovered = Input::is_hovered(pos, size);
	bool pressed = hovered && Input::is_mouse_down(GLFW_MOUSE_BUTTON_LEFT);
	bool clicked = hovered && Input::is_mouse_pressed(GLFW_MOUSE_BUTTON_LEFT);

	glm::vec4 bg = theme.colors.surface;
	glm::vec4 border = theme.colors.border;

	if(pressed) {
		bg = theme.colors.surface_pressed;
		border = theme.colors.accent;
	} else if(hovered) {
		bg = theme.colors.surface_hover;
		border = theme.colors.accent;
	}

	Render::draw_rounded_rect_with_border(pos.x, pos.y, size.x, size.y, theme.style.corner_radius,
	                                      theme.style.border_width, bg, border);

	float textWidth = Render::fontRenderer->measure_text(label);
	float textX = pos.x + (size.x - textWidth) * 0.5f;
	float textY = pos.y + size.y * 0.5f + 6.0f;
	draw_text(label, {textX, textY}, theme.colors.text);

	return clicked;
}

bool button_small(int id, glm::vec2 pos, const std::string &label) {
	float textWidth = Render::fontRenderer->measure_text(label);
	return button(id, pos, {textWidth + 24.0f, 28.0f}, label);
}

bool button_primary(int id, glm::vec2 pos, glm::vec2 size, const std::string &label) {
	auto &theme = Theme::get_current();
	bool hovered = Input::is_hovered(pos, size);
	bool pressed = hovered && Input::is_mouse_down(GLFW_MOUSE_BUTTON_LEFT);
	bool clicked = hovered && Input::is_mouse_pressed(GLFW_MOUSE_BUTTON_LEFT);

	glm::vec4 bg = theme.colors.accent;
	if(pressed) {
		bg = theme.colors.accent_pressed;
	} else if(hovered) {
		bg = theme.colors.accent_hover;
	}

	Render::draw_rounded_rect(pos.x, pos.y, size.x, size.y, theme.style.corner_radius, bg);

	float textWidth = Render::fontRenderer->measure_text(label);
	float textX = pos.x + (size.x - textWidth) * 0.5f;
	float textY = pos.y + size.y * 0.5f + 6.0f;
	draw_text(label, {textX, textY}, {1.0f, 1.0f, 1.0f, 1.0f});

	return clicked;
}

void input_field(int id, std::string &txt, glm::vec2 pos, glm::vec2 size) {
	auto &theme = Theme::get_current();
	bool hovered = Input::is_hovered(pos, size);
	if(Input::is_mouse_pressed(GLFW_MOUSE_BUTTON_LEFT) && hovered) {
		state.focused_id = id;
	}

	if(state.focused_id == id) {
		std::string chars = Input::get_chars();
		txt += chars;

		if(Input::is_key_pressed(GLFW_KEY_BACKSPACE)) {
			if(Input::is_alt_down() && !txt.empty()) {
				size_t p = txt.find_last_of(" \t", txt.length() - 2);
				if(p == std::string::npos) p = 0;
				else p++;
				txt.erase(p);
			} else if(!txt.empty()) {
				txt.pop_back();
			}
		}
	}

	bool focused = state.focused_id == id;
	glm::vec4 border = focused ? theme.colors.border_focused : (hovered ? theme.colors.accent : theme.colors.border);

	Render::draw_rounded_rect_with_border(pos.x, pos.y, size.x, size.y, theme.style.corner_radius,
	                                      theme.style.border_width, theme.colors.surface, border);

	float textStart = pos.x + theme.style.padding;
	float maxWidth = size.x - theme.style.padding * 2.0f;
	std::string displayText = txt.empty() ? "Type here..." : txt;
	glm::vec4 txtColor = txt.empty() ? theme.colors.text_muted : theme.colors.text;
	float lineHeight = Render::fontRenderer->get_line_height();
	float textY = pos.y + (size.y - lineHeight) * 0.5f + lineHeight * 0.75f;

	float textWidth = Render::fontRenderer->measure_text(displayText);
	float &scrollX = state.input_states[id].scroll_offset;

	float cursorX = Render::fontRenderer->measure_text(displayText);
	float visibleWidth = maxWidth;
	float targetScrollX = 0.0f;

	if(textWidth > visibleWidth) {
		targetScrollX = cursorX - visibleWidth + 5.0f;
		targetScrollX = std::max(0.0f, std::min(targetScrollX, textWidth - visibleWidth));
	}

	scrollX += (targetScrollX - scrollX) * 0.3f;
	if(std::abs(targetScrollX - scrollX) < 0.5f) {
		scrollX = targetScrollX;
	}

	float drawX = textStart - scrollX;

	Render::set_scissor(textStart, pos.y, maxWidth, size.y);
	draw_text(displayText, {drawX, textY}, txtColor);

	if(focused) {
		float cursorDrawX = textStart + cursorX - scrollX;
		if(cursorDrawX >= textStart && cursorDrawX <= textStart + visibleWidth) {
			float cursorY = pos.y + (size.y - lineHeight) * 0.5f;
			Render::draw_rect(cursorDrawX, cursorY, 2, lineHeight, theme.colors.accent);
		}
	}

	Render::reset_scissor();
}

void checkbox(int id, bool &checked, glm::vec2 pos, const std::string &label) {
	auto &theme = Theme::get_current();
	float boxSize = 20.0f;
	glm::vec2 size(boxSize, boxSize);
	bool hovered = Input::is_hovered(pos, size);

	if(Input::is_mouse_pressed(GLFW_MOUSE_BUTTON_LEFT) && hovered) {
		checked = !checked;
	}

	glm::vec4 bg = theme.colors.surface;
	glm::vec4 border = theme.colors.border;
	if(hovered) {
		border = theme.colors.accent;
	}

	Render::draw_rounded_rect_with_border(pos.x, pos.y, boxSize, boxSize, 4.0f, theme.style.border_width, bg, border);

	if(checked) {
		float checkMargin = 5.0f;
		Render::draw_rounded_rect(pos.x + checkMargin, pos.y + checkMargin, boxSize - checkMargin * 2.0f,
		                          boxSize - checkMargin * 2.0f, 2.0f, theme.colors.accent);
	}

	draw_text(label, {pos.x + boxSize + 10.0f, pos.y + 14.0f}, theme.colors.text);
}

void slider_float(int id, float &value, float min, float max, glm::vec2 pos, glm::vec2 size) {
	auto &theme = Theme::get_current();
	bool hovered = Input::is_hovered(pos, size);
	static bool dragging = false;
	static int dragId = -1;

	if(Input::is_mouse_pressed(GLFW_MOUSE_BUTTON_LEFT) && hovered) {
		dragging = true;
		dragId = id;
	}

	if(!Input::is_mouse_down(GLFW_MOUSE_BUTTON_LEFT)) {
		dragging = false;
	}

	if(dragging && dragId == id) {
		glm::vec2 mousePos = Input::get_mouse_pos();
		float t = (mousePos.x - pos.x - 8.0f) / (size.x - 16.0f);
		t = std::max(0.0f, std::min(1.0f, t));
		value = min + t * (max - min);
	}

	float trackHeight = 4.0f;
	float trackY = pos.y + (size.y - trackHeight) * 0.5f;
	Render::draw_rounded_rect(pos.x, trackY, size.x, trackHeight, trackHeight * 0.5f, theme.colors.border);

	float t = (value - min) / (max - min);
	Render::draw_rounded_rect(pos.x, trackY, size.x * t, trackHeight, trackHeight * 0.5f, theme.colors.accent);

	float knobSize = 16.0f;
	float knobX = pos.x + t * (size.x - knobSize);
	float knobY = pos.y + (size.y - knobSize) * 0.5f;
	Render::draw_rounded_rect(knobX, knobY, knobSize, knobSize, knobSize * 0.5f,
	                          hovered || (dragging && dragId == id) ? theme.colors.accent_hover : theme.colors.accent);
}

void slider_int(int id, int &value, int min, int max, glm::vec2 pos, glm::vec2 size) {
	float fvalue = (float)value;
	slider_float(id, fvalue, (float)min, (float)max, pos, size);
	value = (int)std::round(fvalue);
}

static std::map<int, bool> dropdownOpen;

bool dropdown(int id, std::string &selected, const std::vector<std::string> &options, glm::vec2 pos, glm::vec2 size) {
	auto &theme = Theme::get_current();
	bool hovered = Input::is_hovered(pos, size);
	bool open = dropdownOpen[id];

	if(Input::is_mouse_pressed(GLFW_MOUSE_BUTTON_LEFT)) {
		if(hovered) {
			dropdownOpen[id] = !open;
		} else if(open) {
			glm::vec2 mousePos = Input::get_mouse_pos();
			float totalHeight = size.y + options.size() * 30.0f;
			if(mousePos.x < pos.x || mousePos.x > pos.x + size.x || mousePos.y < pos.y ||
			   mousePos.y > pos.y + totalHeight) {
				dropdownOpen[id] = false;
			}
		}
	}

	open = dropdownOpen[id];

	glm::vec4 bg = theme.colors.surface;
	glm::vec4 border = hovered ? theme.colors.accent : theme.colors.border;
	Render::draw_rounded_rect_with_border(pos.x, pos.y, size.x, size.y, theme.style.corner_radius,
	                                      theme.style.border_width, bg, border);

	draw_text(selected, {pos.x + 10.0f, pos.y + size.y * 0.5f + 6.0f}, theme.colors.text);

	float arrowX = pos.x + size.x - 20.0f;
	float arrowY = pos.y + size.y * 0.5f;
	draw_text(open ? "▲" : "▼", {arrowX, arrowY + 4.0f}, theme.colors.text_muted);

	if(open) {
		float optionY = pos.y + size.y + 4.0f;
		Render::draw_rounded_rect(pos.x, optionY, size.x, options.size() * 30.0f + 8.0f, theme.style.corner_radius,
		                          theme.colors.surface);

		for(size_t i = 0; i < options.size(); i++) {
			glm::vec2 optPos(pos.x + 4.0f, optionY + 4.0f + i * 30.0f);
			glm::vec2 optSize(size.x - 8.0f, 28.0f);
			bool optHovered = Input::is_hovered(optPos, optSize);

			if(optHovered) {
				Render::draw_rounded_rect(optPos.x, optPos.y, optSize.x, optSize.y, 4.0f, theme.colors.surface_hover);
			}

			draw_text(options[i], {optPos.x + 8.0f, optPos.y + 18.0f}, theme.colors.text);

			if(optHovered && Input::is_mouse_pressed(GLFW_MOUSE_BUTTON_LEFT)) {
				selected = options[i];
				dropdownOpen[id] = false;
				return true;
			}
		}
	}

	return false;
}

void progress_bar(float progress, glm::vec2 pos, glm::vec2 size, glm::vec4 color) {
	auto &theme = Theme::get_current();
	progress = std::max(0.0f, std::min(1.0f, progress));

	Render::draw_rounded_rect(pos.x, pos.y, size.x, size.y, theme.style.corner_radius, theme.colors.border);

	if(progress > 0.0f) {
		Render::draw_rounded_rect(pos.x, pos.y, size.x * progress, size.y, theme.style.corner_radius, color);
	}
}

void separator(glm::vec2 pos, float width) {
	auto &theme = Theme::get_current();
	Render::draw_rect(pos.x, pos.y, width, 1.0f, theme.colors.border);
}

void label(const std::string &txt, glm::vec2 pos, glm::vec4 color) {
	draw_text(txt, pos, color);
}

void label_small(const std::string &txt, glm::vec2 pos, glm::vec4 color) {
	draw_text(txt, pos, color);
}

void panel(glm::vec2 pos, glm::vec2 size, const std::string &title) {
	auto &theme = Theme::get_current();
	Render::draw_rounded_rect_with_border(pos.x, pos.y, size.x, size.y, theme.style.corner_radius,
	                                      theme.style.border_width, theme.colors.surface, theme.colors.border);

	if(!title.empty()) {
		draw_text(title, {pos.x + 16.0f, pos.y + 22.0f}, theme.colors.text);
	}
}

void tooltip(const std::string &txt) {}

void spacing(float height) {}

void update_input_states() {
	state.animation_time += 0.016f;
}

void update() {
	update_input_states();
}

void clear_focus() {
	if(Input::is_mouse_pressed(GLFW_MOUSE_BUTTON_LEFT)) {
		state.focused_id = -1;
	}
}

void draw_rect(float x, float y, float w, float h, glm::vec4 color) {
	Render::draw_rect(x, y, w, h, color);
}

void draw_rounded_rect(float x, float y, float w, float h, float radius, glm::vec4 color) {
	Render::draw_rounded_rect(x, y, w, h, radius, color);
}

} // namespace UI
