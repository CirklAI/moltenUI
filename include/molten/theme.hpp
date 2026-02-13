#pragma once
#include <glm/glm.hpp>

namespace UI {
namespace Theme {

struct Colors {
	glm::vec4 background;
	glm::vec4 surface;
	glm::vec4 surface_hover;
	glm::vec4 surface_pressed;
	glm::vec4 border;
	glm::vec4 border_focused;
	glm::vec4 text;
	glm::vec4 text_muted;
	glm::vec4 accent;
	glm::vec4 accent_hover;
	glm::vec4 accent_pressed;
	glm::vec4 success;
	glm::vec4 warning;
	glm::vec4 error;
};

struct Style {
	float corner_radius;
	float border_width;
	float padding;
	float spacing;
	float shadow_blur;
	glm::vec4 shadow_color;
};

struct ThemeData {
	Colors colors;
	Style style;
};

ThemeData &get_current();
void set_theme(const ThemeData &theme);

ThemeData create_dark_theme();
ThemeData create_purple_theme();
ThemeData create_light_theme();

} // namespace Theme
} // namespace UI
