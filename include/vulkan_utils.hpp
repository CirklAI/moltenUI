#pragma once
#include <functional>
#include <vector>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace VulkanUtils {

struct Buffer {
	VkBuffer buffer;
	VmaAllocation allocation;
	void *mappedData;
	size_t size;
};

struct Image {
	VkImage image;
	VmaAllocation allocation;
	VkImageView view;
	VkFormat format;
	uint32_t width;
	uint32_t height;
};

Buffer createBuffer(VmaAllocator allocator, size_t size, VkBufferUsageFlags usage, VmaMemoryUsage memUsage);
void destroyBuffer(VmaAllocator allocator, Buffer &buffer);

Image createImage(VkDevice device, VmaAllocator allocator, uint32_t width, uint32_t height, VkFormat format,
                  VkImageUsageFlags usage, VmaMemoryUsage memUsage);
void destroyImage(VkDevice device, VmaAllocator allocator, Image &image);

void transitionImageLayout(VkCommandBuffer cmd, VkImage image, VkFormat format, VkImageLayout oldLayout,
                           VkImageLayout newLayout);

void copyBufferToImage(VkCommandBuffer cmd, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

VkShaderModule createShaderModule(VkDevice device, const std::vector<uint32_t> &code);

VkPipelineLayout createPipelineLayout(VkDevice device, VkDescriptorSetLayout *layouts, uint32_t layoutCount);

VkDescriptorSetLayout createDescriptorSetLayout(VkDevice device,
                                                const std::vector<VkDescriptorSetLayoutBinding> &bindings);

VkDescriptorPool createDescriptorPool(VkDevice device, const std::vector<VkDescriptorPoolSize> &poolSizes,
                                      uint32_t maxSets);

void executeOneTimeCommands(VkDevice device, VkCommandPool pool, VkQueue queue,
                            std::function<void(VkCommandBuffer)> func);

} // namespace VulkanUtils
