#include "wrapper.hpp"
#include <molten/molten.hpp>
#include <molten/theme.hpp>
#include <render.hpp>
#include <string>

extern "C" {

MoltenApp molten_app_create(const char *title, int width, int height) {
	try {
		auto *app = new Molten(title, width, height);
		return static_cast<MoltenApp>(app);
	} catch(...) {
		return nullptr;
	}
}

int molten_app_init(MoltenApp app) {
	if(!app) return -1;
	try {
		auto *molten_app = static_cast<Molten *>(app);
		return molten_app->init();
	} catch(...) {
		return -1;
	}
}

static void (*g_callback)(void) = nullptr;

static void callback_wrapper() {
	if(g_callback) {
		g_callback();
	}
}

int molten_app_run(MoltenApp app, void (*callback)(void)) {
	if(!app) return -1;
	try {
		g_callback = callback;
		auto *molten_app = static_cast<Molten *>(app);
		return molten_app->run(callback_wrapper);
	} catch(...) {
		return -1;
	}
}

void molten_app_destroy(MoltenApp app) {
	if(app) {
		delete static_cast<Molten *>(app);
	}
}

void molten_ui_clear_focus() {
	UI::clear_focus();
}

void molten_ui_update() {
	UI::update();
}

void molten_ui_draw_rect(float x, float y, float w, float h, MoltenColor color) {
	UI::draw_rect(x, y, w, h, {color.r, color.g, color.b, color.a});
}

void molten_ui_draw_rounded_rect(float x, float y, float w, float h, float radius, MoltenColor color) {
	UI::draw_rounded_rect(x, y, w, h, radius, {color.r, color.g, color.b, color.a});
}

void molten_ui_panel(MoltenVec2 pos, MoltenVec2 size, const char *title) {
	UI::panel({pos.x, pos.y}, {size.x, size.y}, title);
}

void molten_ui_label(const char *text, MoltenVec2 pos, MoltenColor color) {
	UI::label(text, {pos.x, pos.y}, {color.r, color.g, color.b, color.a});
}

void molten_ui_label_small(const char *text, MoltenVec2 pos, MoltenColor color) {
	UI::label_small(text, {pos.x, pos.y}, {color.r, color.g, color.b, color.a});
}

void molten_ui_separator(MoltenVec2 pos, float width) {
	UI::separator({pos.x, pos.y}, width);
}

bool molten_ui_button(int id, MoltenVec2 pos, MoltenVec2 size, const char *label) {
	return UI::button(id, {pos.x, pos.y}, {size.x, size.y}, label);
}

bool molten_ui_button_primary(int id, MoltenVec2 pos, MoltenVec2 size, const char *label) {
	return UI::button_primary(id, {pos.x, pos.y}, {size.x, size.y}, label);
}

bool molten_ui_checkbox(int id, bool *value, MoltenVec2 pos, const char *label) {
	UI::checkbox(id, *value, {pos.x, pos.y}, label);
	return *value;
}

void molten_ui_slider_float(int id, float *value, float min, float max, MoltenVec2 pos, MoltenVec2 size) {
	UI::slider_float(id, *value, min, max, {pos.x, pos.y}, {size.x, size.y});
}

void molten_ui_slider_int(int id, int *value, int min, int max, MoltenVec2 pos, MoltenVec2 size) {
	UI::slider_int(id, *value, min, max, {pos.x, pos.y}, {size.x, size.y});
}

void molten_ui_progress_bar(float progress, MoltenVec2 pos, MoltenVec2 size, MoltenColor color) {
	UI::progress_bar(progress, {pos.x, pos.y}, {size.x, size.y}, {color.r, color.g, color.b, color.a});
}

MoltenString molten_string_create(const char *str) {
	return new std::string(str);
}

const char *molten_string_get(MoltenString s) {
	if(!s) return "";
	return static_cast<std::string *>(s)->c_str();
}

void molten_string_destroy(MoltenString s) {
	if(s) {
		delete static_cast<std::string *>(s);
	}
}

void molten_ui_input_field(int id, MoltenString str, MoltenVec2 pos, MoltenVec2 size) {
	if(str) {
		auto *cpp_str = static_cast<std::string *>(str);
		UI::input_field(id, *cpp_str, {pos.x, pos.y}, {size.x, size.y});
	}
}

void molten_theme_set_purple() {
	UI::Theme::set_theme(UI::Theme::create_purple_theme());
}

void molten_theme_get_colors(MoltenThemeColors *colors) {
	if(!colors) return;

	auto &theme = UI::Theme::get_current();
	colors->background = {theme.colors.background.r, theme.colors.background.g, theme.colors.background.b,
	                      theme.colors.background.a};
	colors->panel = {theme.colors.background.r, theme.colors.background.g, theme.colors.background.b,
	                 theme.colors.background.a};
	colors->border = {theme.colors.border.r, theme.colors.border.g, theme.colors.border.b, theme.colors.border.a};
	colors->text = {theme.colors.text.r, theme.colors.text.g, theme.colors.text.b, theme.colors.text.a};
	colors->text_muted = {theme.colors.text_muted.r, theme.colors.text_muted.g, theme.colors.text_muted.b,
	                      theme.colors.text_muted.a};
	colors->accent = {theme.colors.accent.r, theme.colors.accent.g, theme.colors.accent.b, theme.colors.accent.a};
	colors->success = {theme.colors.success.r, theme.colors.success.g, theme.colors.success.b, theme.colors.success.a};
	colors->error = {theme.colors.error.r, theme.colors.error.g, theme.colors.error.b, theme.colors.error.a};
	colors->warning = {theme.colors.warning.r, theme.colors.warning.g, theme.colors.warning.b, theme.colors.warning.a};
}

void molten_shaders_init_with_data(const uint32_t *rect_spirv, size_t rect_size, const uint32_t *rounded_rect_spirv,
                                   size_t rounded_rect_size) {
	std::vector<uint32_t> rectData(rect_spirv, rect_spirv + rect_size / sizeof(uint32_t));
	std::vector<uint32_t> roundedData(rounded_rect_spirv, rounded_rect_spirv + rounded_rect_size / sizeof(uint32_t));
	Render::init_shaders_with_data(rectData, roundedData);
}

} // extern "C"
