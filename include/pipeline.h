#ifndef PIPELINE_H
#define PIPELINE_H

#include <vulkan/vulkan.h>

typedef struct {
	VkPipeline pipeline;
	VkPipelineLayout layout;
	VkDescriptorSetLayout descriptor_set_layout;
} PipelineContext;

PipelineContext create_graphics_pipeline(VkDevice dev, VkRenderPass render_pass, VkExtent2D extent);

#endif
