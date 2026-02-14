#ifndef MOLTEN_WRAPPER_HPP
#define MOLTEN_WRAPPER_HPP

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void *MoltenApp;
typedef void *MoltenString;

typedef struct {
	float r, g, b, a;
} MoltenColor;

typedef struct {
	float x, y;
} MoltenVec2;

MoltenApp molten_app_create(const char *title, int width, int height);
int molten_app_init(MoltenApp app);
int molten_app_run(MoltenApp app, void (*callback)(void));
void molten_app_destroy(MoltenApp app);

void molten_ui_clear_focus();
void molten_ui_update();
void molten_ui_draw_rect(float x, float y, float w, float h, MoltenColor color);
void molten_ui_draw_rounded_rect(float x, float y, float w, float h, float radius, MoltenColor color);

void molten_ui_panel(MoltenVec2 pos, MoltenVec2 size, const char *title);
void molten_ui_label(const char *text, MoltenVec2 pos, MoltenColor color);
void molten_ui_label_small(const char *text, MoltenVec2 pos, MoltenColor color);
void molten_ui_separator(MoltenVec2 pos, float width);

bool molten_ui_button(int id, MoltenVec2 pos, MoltenVec2 size, const char *label);
bool molten_ui_button_primary(int id, MoltenVec2 pos, MoltenVec2 size, const char *label);
bool molten_ui_checkbox(int id, bool *value, MoltenVec2 pos, const char *label);

void molten_ui_slider_float(int id, float *value, float min, float max, MoltenVec2 pos, MoltenVec2 size);
void molten_ui_slider_int(int id, int *value, int min, int max, MoltenVec2 pos, MoltenVec2 size);
void molten_ui_progress_bar(float progress, MoltenVec2 pos, MoltenVec2 size, MoltenColor color);

MoltenString molten_string_create(const char *str);
const char *molten_string_get(MoltenString s);
void molten_string_destroy(MoltenString s);
void molten_ui_input_field(int id, MoltenString str, MoltenVec2 pos, MoltenVec2 size);

typedef struct {
	MoltenColor background;
	MoltenColor panel;
	MoltenColor border;
	MoltenColor text;
	MoltenColor text_muted;
	MoltenColor accent;
	MoltenColor success;
	MoltenColor error;
	MoltenColor warning;
} MoltenThemeColors;

void molten_theme_set_purple();
void molten_theme_get_colors(MoltenThemeColors *colors);

void molten_shaders_init_with_data(const uint32_t *rect_spirv, size_t rect_size, const uint32_t *rounded_rect_spirv,
                                   size_t rounded_rect_size);

#ifdef __cplusplus
}
#endif

#endif
