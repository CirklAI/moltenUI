#include "vulkan_utils.hpp"
#include <stdexcept>

namespace VulkanUtils {

Buffer createBuffer(VmaAllocator allocator, size_t size, VkBufferUsageFlags usage, VmaMemoryUsage memUsage) {
	Buffer buf;
	buf.size = size;

	VkBufferCreateInfo bci{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .size = size, .usage = usage};

	VmaAllocationCreateInfo aci{.usage = memUsage};

	if(memUsage == VMA_MEMORY_USAGE_CPU_ONLY || memUsage == VMA_MEMORY_USAGE_CPU_TO_GPU) {
		aci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
	}

	VmaAllocationInfo allocInfo;
	vmaCreateBuffer(allocator, &bci, &aci, &buf.buffer, &buf.allocation, &allocInfo);
	buf.mappedData = allocInfo.pMappedData;

	return buf;
}

void destroyBuffer(VmaAllocator allocator, Buffer &buffer) {
	vmaDestroyBuffer(allocator, buffer.buffer, buffer.allocation);
	buffer.buffer = VK_NULL_HANDLE;
	buffer.allocation = VK_NULL_HANDLE;
	buffer.mappedData = nullptr;
}

Image createImage(VkDevice device, VmaAllocator allocator, uint32_t width, uint32_t height, VkFormat format,
                  VkImageUsageFlags usage, VmaMemoryUsage memUsage) {
	Image img;
	img.format = format;
	img.width = width;
	img.height = height;

	VkImageCreateInfo ici{.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
	                      .imageType = VK_IMAGE_TYPE_2D,
	                      .format = format,
	                      .extent = {width, height, 1},
	                      .mipLevels = 1,
	                      .arrayLayers = 1,
	                      .samples = VK_SAMPLE_COUNT_1_BIT,
	                      .tiling = VK_IMAGE_TILING_OPTIMAL,
	                      .usage = usage};

	VmaAllocationCreateInfo aci{.usage = memUsage};
	vmaCreateImage(allocator, &ici, &aci, &img.image, &img.allocation, nullptr);

	VkImageViewCreateInfo vci{.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
	                          .image = img.image,
	                          .viewType = VK_IMAGE_VIEW_TYPE_2D,
	                          .format = format,
	                          .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};
	vkCreateImageView(device, &vci, nullptr, &img.view);

	return img;
}

void destroyImage(VkDevice device, VmaAllocator allocator, Image &image) {
	vkDestroyImageView(device, image.view, nullptr);
	vmaDestroyImage(allocator, image.image, image.allocation);
	image.image = VK_NULL_HANDLE;
	image.allocation = VK_NULL_HANDLE;
	image.view = VK_NULL_HANDLE;
}

void transitionImageLayout(VkCommandBuffer cmd, VkImage image, VkFormat format, VkImageLayout oldLayout,
                           VkImageLayout newLayout) {
	VkImageMemoryBarrier barrier{.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
	                             .oldLayout = oldLayout,
	                             .newLayout = newLayout,
	                             .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
	                             .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
	                             .image = image,
	                             .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};

	VkPipelineStageFlags srcStage, dstStage;

	if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} else if(oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
	          newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} else {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = 0;
		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	}

	vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void copyBufferToImage(VkCommandBuffer cmd, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
	VkBufferImageCopy region{.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
	                         .imageExtent = {width, height, 1}};
	vkCmdCopyBufferToImage(cmd, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

VkShaderModule createShaderModule(VkDevice device, const std::vector<uint32_t> &code) {
	VkShaderModuleCreateInfo ci{.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
	                            .codeSize = code.size() * sizeof(uint32_t),
	                            .pCode = code.data()};
	VkShaderModule mod;
	if(vkCreateShaderModule(device, &ci, nullptr, &mod) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create shader module");
	}
	return mod;
}

VkPipelineLayout createPipelineLayout(VkDevice device, VkDescriptorSetLayout *layouts, uint32_t layoutCount) {
	VkPipelineLayoutCreateInfo ci{
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, .setLayoutCount = layoutCount, .pSetLayouts = layouts};
	VkPipelineLayout layout;
	vkCreatePipelineLayout(device, &ci, nullptr, &layout);
	return layout;
}

VkDescriptorSetLayout createDescriptorSetLayout(VkDevice device,
                                                const std::vector<VkDescriptorSetLayoutBinding> &bindings) {
	VkDescriptorSetLayoutCreateInfo ci{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
	                                   .bindingCount = (uint32_t)bindings.size(),
	                                   .pBindings = bindings.data()};
	VkDescriptorSetLayout layout;
	vkCreateDescriptorSetLayout(device, &ci, nullptr, &layout);
	return layout;
}

VkDescriptorPool createDescriptorPool(VkDevice device, const std::vector<VkDescriptorPoolSize> &poolSizes,
                                      uint32_t maxSets) {
	VkDescriptorPoolCreateInfo ci{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
	                              .maxSets = maxSets,
	                              .poolSizeCount = (uint32_t)poolSizes.size(),
	                              .pPoolSizes = poolSizes.data()};
	VkDescriptorPool pool;
	vkCreateDescriptorPool(device, &ci, nullptr, &pool);
	return pool;
}

void executeOneTimeCommands(VkDevice device, VkCommandPool pool, VkQueue queue,
                            std::function<void(VkCommandBuffer)> func) {
	VkCommandBufferAllocateInfo ai{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
	                               .commandPool = pool,
	                               .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
	                               .commandBufferCount = 1};
	VkCommandBuffer cmd;
	vkAllocateCommandBuffers(device, &ai, &cmd);

	VkCommandBufferBeginInfo bi{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	                            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};
	vkBeginCommandBuffer(cmd, &bi);

	func(cmd);

	vkEndCommandBuffer(cmd);

	VkSubmitInfo si{.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO, .commandBufferCount = 1, .pCommandBuffers = &cmd};
	vkQueueSubmit(queue, 1, &si, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);

	vkFreeCommandBuffers(device, pool, 1, &cmd);
}

} // namespace VulkanUtils
