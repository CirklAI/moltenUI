#pragma once
#include <ft2build.h>
#include FT_FREETYPE_H
#include <glm/glm.hpp>
#include <map>
#include <string>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

struct Glyph {
	glm::vec2 size;
	glm::vec2 bearing;
	uint32_t advance;
	glm::vec4 uvRect;
};

class FontRenderer {
  public:
	FontRenderer(const std::string &fontPath, uint32_t fontSize);
	~FontRenderer();

	void draw_text(VkCommandBuffer cmd, const std::string &text, glm::vec2 pos, glm::vec4 color);

	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;
	VkDescriptorSet descriptorSet;

  private:
	std::map<char, Glyph> glyphs;
	VkImage atlasImage;
	VmaAllocation atlasAllocation;
	VkImageView atlasView;
	VkSampler atlasSampler;
	VkDescriptorSetLayout descLayout;
	VkDescriptorPool descPool;

	void create_atlas(const std::string &path, uint32_t fontSize);
};
