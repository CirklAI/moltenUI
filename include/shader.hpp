#pragma once
#include <string>
#include <vulkan/vulkan.h>

class Shader {
  public:
	VkPipeline pipeline;
	VkPipelineLayout layout;
	VkShaderModule vertModule;
	VkShaderModule fragModule;

	Shader(const std::string &vertPath, const std::string &fragPath);
	~Shader();

	void use(VkCommandBuffer cmd);
};
