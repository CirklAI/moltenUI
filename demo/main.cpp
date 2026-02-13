#include "molten/theme.hpp"
#include <ctime>
#include <iostream>
#include <molten/molten.hpp>

float fps = 60.0f;
int frame_count = 0;
float last_time = 0.0f;

void demo() {
	UI::clear_focus();
	auto &theme = UI::Theme::get_current();
	UI::draw_rect(0, 0, 1280, 720, theme.colors.background);

	frame_count++;
	float now = (float)clock() / CLOCKS_PER_SEC;
	if(now - last_time >= 1.0f) {
		fps = (float)frame_count;
		frame_count = 0;
		last_time = now;
	}

	const float col_w = 220.0f;
	const float gap = 20.0f;
	const float col1_x = 20.0f;
	const float col2_x = col1_x + col_w + gap;
	const float col3_x = col2_x + col_w + gap;
	const float col4_x = col3_x + col_w + gap;
	const float start_y = 20.0f;
	float y = start_y;

	y = start_y;
	UI::panel({col1_x, y}, {col_w, 230}, "Sign In");
	y += 45.0f;
	static std::string email_str = "";
	UI::input_field(1, email_str, {col1_x + 15, y}, {col_w - 30, 36});
	y += 45.0f;
	static std::string pass_str = "";
	UI::input_field(2, pass_str, {col1_x + 15, y}, {col_w - 30, 36});
	y += 45.0f;
	static bool remember_me = true;
	UI::checkbox(3, remember_me, {col1_x + 15, y}, "Remember me");
	y += 50.0f;
	UI::button_primary(4, {col1_x + 15, y}, {col_w - 30, 36}, "Sign In");

	y = 250.0f + 25.0f;
	UI::panel({col1_x, y}, {col_w, 140}, "Toggles");
	y += 45.0f;
	static bool t1 = true, t2 = false, t3 = true;
	UI::checkbox(10, t1, {col1_x + 15, y}, "Enabled");
	y += 30.0f;
	UI::checkbox(11, t2, {col1_x + 15, y}, "Notifications");
	y += 30.0f;
	UI::checkbox(12, t3, {col1_x + 15, y}, "Auto-save");

	y = start_y;
	UI::panel({col2_x, y}, {col_w, 235}, "Buttons");
	y += 45.0f;
	UI::button_primary(20, {col2_x + 15, y}, {col_w - 30, 32}, "Submit");
	y += 42.0f;
	UI::button(21, {col2_x + 15, y}, {col_w - 30, 32}, "Cancel");
	y += 42.0f;
	UI::button(22, {col2_x + 15, y}, {col_w - 30, 32}, "Delete");
	y += 42.0f;
	UI::button(23, {col2_x + 15, y}, {90, 28}, "Edit");
	UI::button(24, {col2_x + 115, y}, {90, 28}, "Copy");

	y = 255.0f + 25.0f;
	UI::panel({col2_x, y}, {col_w, 145}, "Sliders");
	y += 45.0f;
	static float vol = 75.0f;
	UI::label(std::to_string((int)vol) + "%", {col2_x + 15, y + 12}, theme.colors.text_muted);
	UI::slider_float(14, vol, 0.0f, 100.0f, {col2_x + 55, y}, {col_w - 75, 24});
	y += 45.0f;
	static float bright = 50.0f;
	UI::slider_float(15, bright, 0.0f, 100.0f, {col2_x + 15, y}, {col_w - 30, 16});
	y += 28.0f;
	UI::progress_bar(bright / 100.0f, {col2_x + 15, y}, {col_w - 30, 10}, theme.colors.accent);

	y = start_y;
	UI::panel({col3_x, y}, {col_w, 260}, "Info");
	y += 45.0f;
	UI::label("moltenUI", {col3_x + 15, y}, theme.colors.accent);
	y += 24.0f;
	UI::label_small("v1.0.0 (Vulkan Backend)", {col3_x + 15, y}, theme.colors.text_muted);
	y += 24.0f;
	UI::separator({col3_x + 15, y}, col_w - 30);
	y += 20.0f;
	UI::label_small("- Rounded corners", {col3_x + 15, y}, theme.colors.text_muted);
	y += 18.0f;
	UI::label_small("- Theme support", {col3_x + 15, y}, theme.colors.text_muted);
	y += 24.0f;
	UI::separator({col3_x + 15, y}, col_w - 30);
	y += 20.0f;
	UI::label("FPS: " + std::to_string((int)fps), {col3_x + 15, y}, theme.colors.text);

	y = 280.0f + 25.0f;
	UI::panel({col3_x, y}, {col_w, 165}, "RGB Color");
	y += 45.0f;
	static float rv = 200, gv = 50, bv = 255;
	UI::slider_float(40, rv, 0.0f, 255.0f, {col3_x + 15, y}, {col_w - 30, 14});
	y += 22.0f;
	UI::slider_float(41, gv, 0.0f, 255.0f, {col3_x + 15, y}, {col_w - 30, 14});
	y += 22.0f;
	UI::slider_float(42, bv, 0.0f, 255.0f, {col3_x + 15, y}, {col_w - 30, 14});
	y += 26.0f;
	UI::draw_rounded_rect(col3_x + 15, y, col_w - 30, 30, 4.0f, {rv / 255.f, gv / 255.f, bv / 255.f, 1.f});

	y = start_y;
	UI::panel({col4_x, y}, {col_w, 400}, "All Widgets");
	y += 45.0f;
	UI::label("Labels:", {col4_x + 15, y}, theme.colors.text);
	y += 24.0f;
	UI::label_small("Small label", {col4_x + 15, y}, theme.colors.text_muted);
	y += 30.0f;
	UI::separator({col4_x + 15, y}, col_w - 30);
	y += 20.0f;
	static bool sw = true;
	UI::checkbox(60, sw, {col4_x + 15, y}, "Switch");
	y += 35.0f;
	UI::separator({col4_x + 15, y}, col_w - 30);
	y += 20.0f;
	static int count = 5;
	UI::slider_int(61, count, 0, 10, {col4_x + 15, y}, {col_w - 30, 16});
	y += 45.0f;
	UI::label("Count: " + std::to_string(count), {col4_x + 15, y}, theme.colors.text_muted);
	y += 35.0f;
	UI::separator({col4_x + 15, y}, col_w - 30);
	y += 20.0f;
	UI::draw_rounded_rect(col4_x + 15, y, 50, 30, 6.0f, theme.colors.error);
	UI::draw_rounded_rect(col4_x + 75, y, 50, 30, 6.0f, theme.colors.success);
	UI::draw_rounded_rect(col4_x + 135, y, 50, 30, 6.0f, theme.colors.warning);
	y += 45.0f;
	UI::draw_rect(col4_x + 15, y, col_w - 30, 15, theme.colors.border);

	UI::update();
}

int main() {
	try {
		UI::Theme::set_theme(UI::Theme::create_purple_theme());
		Molten app("MoltenUI Demo", 1280, 720);
		if(app.init() != 0) return 1;
		return app.run(demo);
	} catch(const std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
}
