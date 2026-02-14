#pragma once
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

class Shader {
  public:
	VkPipeline pipeline;
	VkPipelineLayout layout;
	VkShaderModule vertModule;
	VkShaderModule fragModule;
	VkDescriptorSetLayout descSetLayout;

	Shader(const std::string &shaderPath);
	Shader(const std::vector<uint32_t> &spirvData);
	~Shader();

	void use(VkCommandBuffer cmd);
};
