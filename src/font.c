#include "font.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <stdlib.h>
#include <string.h>

void update_text_buffer(FontAtlas *atlas, const char *text, float x, float y, void *mapped_data) {
	TextVertex *vertices = (TextVertex *)mapped_data;
	int n = 0;

	for(const char *p = text; *p; p++) {
		unsigned char c = (unsigned char)*p;
		GlyphMetric g = atlas->glyphs[c];

		float x2 = x + g.bl;
		float y2 = y - g.bt;
		float w = g.bw;
		float h = g.bh;

		x += g.ax;

		if(w == 0 || h == 0) continue;

		vertices[n++] = (TextVertex){x2, y2, g.tx, g.ty};
		vertices[n++] = (TextVertex){x2 + w, y2, g.tx + g.tw, g.ty};
		vertices[n++] = (TextVertex){x2, y2 + h, g.tx, g.ty + g.th};

		vertices[n++] = (TextVertex){x2 + w, y2, g.tx + g.tw, g.ty};
		vertices[n++] = (TextVertex){x2 + w, y2 + h, g.tx + g.tw, g.ty + g.th};
		vertices[n++] = (TextVertex){x2, y2 + h, g.tx, g.ty + g.th};
	}
}

FontAtlas *create_font_atlas(const char *font_path, int size) {
	FT_Library ft;
	FT_Face face;
	if(FT_Init_FreeType(&ft) || FT_New_Face(ft, font_path, 0, &face)) return NULL;
	FT_Set_Pixel_Sizes(face, 0, size);

	FontAtlas *atlas = calloc(1, sizeof(FontAtlas));
	int atlas_w = 512, atlas_h = 512;
	atlas->pixels = calloc(atlas_w * atlas_h, 1);
	atlas->width = atlas_w;
	atlas->height = atlas_h;

	int x = 0, y = 0, row_h = 0;

	for(int i = 32; i < 128; i++) {
		if(FT_Load_Char(face, i, FT_LOAD_RENDER)) continue;

		FT_GlyphSlot g = face->glyph;

		if(x + (int)g->bitmap.width >= atlas_w) {
			x = 0;
			y += row_h;
			row_h = 0;
		}

		atlas->glyphs[i].ax = (float)(g->advance.x >> 6);
		atlas->glyphs[i].ay = (float)(g->advance.y >> 6);

		atlas->glyphs[i].tx = (float)x / atlas_w;
		atlas->glyphs[i].ty = (float)y / atlas_h;

		for(unsigned int r = 0; r < g->bitmap.rows; r++) {
			memcpy(&atlas->pixels[(y + r) * atlas_w + x], &g->bitmap.buffer[r * g->bitmap.width], g->bitmap.width);
		}

		atlas->glyphs[i].bw = (float)g->bitmap.width;
		atlas->glyphs[i].bh = (float)g->bitmap.rows;
		atlas->glyphs[i].bl = (float)g->bitmap_left;
		atlas->glyphs[i].bt = (float)g->bitmap_top;
		atlas->glyphs[i].tw = (float)g->bitmap.width / atlas_w;
		atlas->glyphs[i].th = (float)g->bitmap.rows / atlas_h;

		x += g->bitmap.width + 1;

		if((int)g->bitmap.rows > row_h) row_h = (int)g->bitmap.rows;
	}

	FT_Done_Face(face);
	FT_Done_FreeType(ft);
	return atlas;
}
