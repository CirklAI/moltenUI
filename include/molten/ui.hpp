#pragma once
#include "molten/theme.hpp"
#include <map>
#include <string>
#include <vector>

namespace UI {

struct InputState {
	float scroll_offset = 0.0f;
	float target_scroll_offset = 0.0f;
};

struct State {
	int focused_id = -1;
	std::map<int, InputState> input_states;
	int hot_id = -1;
	int active_id = -1;
	float animation_time = 0.0f;
};

inline State state;

void update_input_states();

bool button(int id, glm::vec2 pos, glm::vec2 size, const std::string &label);
bool button_small(int id, glm::vec2 pos, const std::string &label);
bool button_primary(int id, glm::vec2 pos, glm::vec2 size, const std::string &label);

void input_field(int id, std::string &text, glm::vec2 pos, glm::vec2 size);

void checkbox(int id, bool &checked, glm::vec2 pos, const std::string &label);

void slider_float(int id, float &value, float min, float max, glm::vec2 pos, glm::vec2 size);
void slider_int(int id, int &value, int min, int max, glm::vec2 pos, glm::vec2 size);

bool dropdown(int id, std::string &selected, const std::vector<std::string> &options, glm::vec2 pos, glm::vec2 size);

void progress_bar(float progress, glm::vec2 pos, glm::vec2 size, glm::vec4 color);

void separator(glm::vec2 pos, float width);

void label(const std::string &text, glm::vec2 pos, glm::vec4 color = Theme::get_current().colors.text);
void label_small(const std::string &text, glm::vec2 pos, glm::vec4 color = Theme::get_current().colors.text_muted);

void panel(glm::vec2 pos, glm::vec2 size, const std::string &title = "");

void tooltip(const std::string &text);

void spacing(float height);

void draw_rect(float x, float y, float w, float h, glm::vec4 color);
void draw_rounded_rect(float x, float y, float w, float h, float radius, glm::vec4 color);

void draw_text(const std::string &label, glm::vec2 pos, glm::vec4 color);

void update();

void clear_focus();

} // namespace UI
