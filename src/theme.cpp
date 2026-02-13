#include "molten/theme.hpp"

namespace UI {
namespace Theme {

static ThemeData current_theme = create_purple_theme();

ThemeData &get_current() {
	return current_theme;
}

void set_theme(const ThemeData &theme) {
	current_theme = theme;
}

ThemeData create_dark_theme() {
	ThemeData t;
	t.colors.background = {0.0f, 0.0f, 0.0f, 1.0f};
	t.colors.surface = {0.05f, 0.05f, 0.05f, 1.0f};
	t.colors.surface_hover = {0.08f, 0.08f, 0.08f, 1.0f};
	t.colors.surface_pressed = {0.03f, 0.03f, 0.03f, 1.0f};
	t.colors.border = {0.15f, 0.15f, 0.15f, 1.0f};
	t.colors.border_focused = {0.35f, 0.35f, 0.35f, 1.0f};
	t.colors.text = {1.0f, 1.0f, 1.0f, 1.0f};
	t.colors.text_muted = {0.5f, 0.5f, 0.5f, 1.0f};
	t.colors.accent = {0.2f, 0.2f, 0.2f, 1.0f};
	t.colors.accent_hover = {0.25f, 0.25f, 0.25f, 1.0f};
	t.colors.accent_pressed = {0.1f, 0.1f, 0.1f, 1.0f};
	t.colors.success = {0.0f, 0.45f, 1.0f, 1.0f};
	t.colors.warning = {1.0f, 0.8f, 0.0f, 1.0f};
	t.colors.error = {1.0f, 0.0f, 0.0f, 1.0f};
	t.style.corner_radius = 5.0f;
	t.style.border_width = 1.0f;
	t.style.padding = 12.0f;
	t.style.spacing = 8.0f;
	return t;
}

ThemeData create_purple_theme() {
	ThemeData t = create_dark_theme();
	t.colors.border = {0.18f, 0.18f, 0.18f, 1.0f};
	t.colors.border_focused = {0.55f, 0.25f, 0.95f, 1.0f};
	t.colors.accent = {0.45f, 0.20f, 0.85f, 1.0f};
	t.colors.accent_hover = {0.55f, 0.25f, 0.95f, 1.0f};
	t.colors.accent_pressed = {0.35f, 0.15f, 0.70f, 1.0f};
	return t;
}

} // namespace Theme
} // namespace UI
