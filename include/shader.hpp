#pragma once
#include <string>
#include <vulkan/vulkan.h>

class Shader {
  public:
	Shader(const std::string &vertPath, const std::string &fragPath);
	~Shader();

	void use(VkCommandBuffer cmd);
	VkPipelineLayout layout;
	VkPipeline pipeline;

  private:
	VkShaderModule createModule(const std::string &filename);
};
