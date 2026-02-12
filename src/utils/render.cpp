#include "utils/render.hpp"
#include "init.hpp"

namespace RenderUtils {

void draw_rect(glm::vec2 position, glm::vec2 size) {
	struct PushConstants {
		glm::vec2 pos;
		glm::vec2 size;
	} constants{position, size};

	vkCmdPushConstants(Init::commandBuffers[Init::currentFrame], Init::pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
	                   sizeof(PushConstants), &constants);

	vkCmdDraw(Init::commandBuffers[Init::currentFrame], 6, 1, 0, 0);
}

} // namespace RenderUtils
