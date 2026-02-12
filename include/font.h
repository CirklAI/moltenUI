#ifndef FONT_H
#define FONT_H

#include <ft2build.h>
#include FT_FREETYPE_H

typedef struct {
	float x, y, u, v;
} TextVertex;

typedef struct {
	float ax; // advance.x
	float ay; // advance.y
	float bw; // bitmap.width
	float bh; // bitmap.height
	float bl; // bitmap_left
	float bt; // bitmap_top
	float tx; // x offset of glyph in texture cords
	float ty; // y offset of glyph in texture cords
	float tw; // width of glyph in texture cords
	float th; // height of glyph in texture cords
} GlyphMetric;

typedef struct {
	GlyphMetric glyphs[128];
	unsigned char *pixels;
	int width;
	int height;
} FontAtlas;

FontAtlas *create_font_atlas(const char *font_path, int size);
void update_text_buffer(FontAtlas *atlas, const char *text, float x, float y, void *mapped_data);

#endif
