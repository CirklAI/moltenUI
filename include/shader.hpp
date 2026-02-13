#pragma once
#include <string>
#include <vulkan/vulkan.h>

class Shader {
  public:
	VkPipeline pipeline;
	VkPipelineLayout layout;
	VkShaderModule vertModule;
	VkShaderModule fragModule;
	VkDescriptorSetLayout descSetLayout;

	Shader(const std::string &shaderPath);
	~Shader();

	void use(VkCommandBuffer cmd);
};
