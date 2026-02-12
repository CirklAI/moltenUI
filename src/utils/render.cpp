#include "utils/render.hpp"
#include "init.hpp"
#include "render.hpp"
#include "shader.hpp"

namespace RenderUtils {

void draw_rect(glm::vec2 position, glm::vec2 size, glm::vec4 color) {
	struct PushConstants {
		glm::vec2 pos;
		glm::vec2 size;
		glm::vec4 color;
	} constants{position, size, color};

	vkCmdPushConstants(Init::commandBuffers[Init::currentFrame], Render::rectShader->layout,
	                   VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstants), &constants);

	vkCmdDraw(Init::commandBuffers[Init::currentFrame], 6, 1, 0, 0);
}

} // namespace RenderUtils
