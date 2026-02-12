#pragma once
#include "font.hpp"
#include "shader.hpp"
#include <memory>

namespace Render {
extern std::unique_ptr<Shader> rectShader;
extern std::unique_ptr<FontRenderer> fontRenderer;
void draw_frame();
void cleanup();
} // namespace Render
