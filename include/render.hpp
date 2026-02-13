#pragma once

#include "font.hpp"
#include <functional>
#include <glm/glm.hpp>

namespace Render {
extern std::unique_ptr<FontRenderer> fontRenderer;

void draw_frame(std::function<void()> callback);
void draw_rect(float x, float y, float w, float h, glm::vec4 color);
void set_scissor(float x, float y, float w, float h);
void reset_scissor();
void cleanup();
} // namespace Render
