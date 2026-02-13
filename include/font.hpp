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
	float measure_text(const std::string &text);
	float get_line_height() const {
		return lineHeight;
	}
	const Glyph *get_glyph(char c) const;

	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;
	VkDescriptorSet descriptorSet;

  public:
	void reset_char_counter();

  private:
	std::map<char, Glyph> glyphs;
	float lineHeight;
	VkImage atlasImage;
	VmaAllocation atlasAllocation;
	VkImageView atlasView;
	VkSampler atlasSampler;
	VkDescriptorSetLayout descLayout;
	VkDescriptorPool descPool;
	static const int MAX_FRAMES = 3;
	static const int MAX_CHARS_PER_FRAME = 256;
	VkBuffer uniformBuffers[MAX_FRAMES][MAX_CHARS_PER_FRAME];
	VmaAllocation uniformAllocs[MAX_FRAMES][MAX_CHARS_PER_FRAME];
	void *uniformMapped[MAX_FRAMES][MAX_CHARS_PER_FRAME];
	VkDescriptorSet descriptorSets[MAX_FRAMES][MAX_CHARS_PER_FRAME];
	int charCounters[MAX_FRAMES] = {0, 0, 0};

	void create_atlas(const std::string &path, uint32_t fontSize);
};
