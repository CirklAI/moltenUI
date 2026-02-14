#pragma once

#include "font.hpp"
#include <functional>
#include <glm/glm.hpp>
#include <vector>

namespace Render {

extern std::unique_ptr<FontRenderer> fontRenderer;

void init_shaders_with_data(const std::vector<uint32_t> &rectSpirv, const std::vector<uint32_t> &roundedRectSpirv);
void draw_frame(std::function<void()> callback);
void draw_rect(float x, float y, float w, float h, glm::vec4 color);
void draw_rounded_rect(float x, float y, float w, float h, float radius, glm::vec4 color);
void draw_rounded_rect_with_border(float x, float y, float w, float h, float radius, float border_width,
                                   glm::vec4 color, glm::vec4 border_color);
void set_scissor(float x, float y, float w, float h);
void reset_scissor();
void cleanup();

} // namespace Render
