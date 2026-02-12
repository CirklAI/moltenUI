// clang-format off
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
// clang-format on
#include "init.hpp"
#include "VkBootstrap.h"
#include "render.hpp"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include <stdexcept>
#include <vector>

namespace Init {

vkb::Instance vkbInstance;
vkb::PhysicalDevice vkbPhysicalDevice;
vkb::Device vkbDevice;
vkb::Swapchain vkbSwapchain;

VkInstance instance;
VkPhysicalDevice physicalDevice;
VkDevice device;
VkSurfaceKHR surface;
VkQueue graphicsQueue;
VkQueue presentQueue;
VkRenderPass renderPass;
VkPipelineLayout pipelineLayout;
VkPipeline graphicsPipeline;

VmaAllocator allocator;

std::vector<VkImage> swapchainImages;
std::vector<VkImageView> swapchainImageViews;
std::vector<VkFramebuffer> framebuffers;
std::vector<VkCommandBuffer> commandBuffers;
VkCommandPool commandPool;

std::vector<VkSemaphore> imageAvailableSemaphores;
std::vector<VkSemaphore> renderFinishedSemaphores;
std::vector<VkFence> inFlightFences;
size_t currentFrame = 0;

void init_vulkan(const char *title, GLFWwindow *window) {
	vkb::InstanceBuilder builder;
	auto inst_ret = builder.set_app_name(title)
	                    .request_validation_layers(true)
	                    .use_default_debug_messenger()
	                    .require_api_version(1, 3, 0)
	                    .build();

	if(!inst_ret) {
		throw std::runtime_error("Failed to create Vulkan instance: " + inst_ret.error().message());
	}

	vkbInstance = inst_ret.value();
	instance = vkbInstance.instance;

	if(glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create window surface!");
	}

	vkb::PhysicalDeviceSelector selector{vkbInstance};
	auto phys_ret = selector.set_surface(surface).set_minimum_version(1, 3).select();

	if(!phys_ret) {
		throw std::runtime_error("Failed to select physical device: " + phys_ret.error().message());
	}

	vkbPhysicalDevice = phys_ret.value();
	physicalDevice = vkbPhysicalDevice.physical_device;

	vkb::DeviceBuilder deviceBuilder{vkbPhysicalDevice};
	auto dev_ret = deviceBuilder.build();

	if(!dev_ret) {
		throw std::runtime_error("Failed to create logical device: " + dev_ret.error().message());
	}

	vkbDevice = dev_ret.value();
	device = vkbDevice.device;

	auto graphics_queue_ret = vkbDevice.get_queue(vkb::QueueType::graphics);
	if(!graphics_queue_ret) {
		throw std::runtime_error("Failed to get graphics queue");
	}
	graphicsQueue = graphics_queue_ret.value();

	auto present_queue_ret = vkbDevice.get_queue(vkb::QueueType::present);
	if(!present_queue_ret) {
		throw std::runtime_error("Failed to get present queue");
	}
	presentQueue = present_queue_ret.value();

	vkb::SwapchainBuilder swapchainBuilder{vkbDevice};
	auto swap_ret = swapchainBuilder.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
	                    .set_desired_format({VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
	                    .build();

	if(!swap_ret) {
		throw std::runtime_error("Failed to create swapchain: " + swap_ret.error().message());
	}

	vkbSwapchain = swap_ret.value();
	swapchainImages = vkbSwapchain.get_images().value();
	swapchainImageViews = vkbSwapchain.get_image_views().value();

	VmaAllocatorCreateInfo allocatorInfo{};
	allocatorInfo.physicalDevice = physicalDevice;
	allocatorInfo.device = device;
	allocatorInfo.instance = instance;
	allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;

	if(vmaCreateAllocator(&allocatorInfo, &allocator) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create VMA allocator!");
	}

	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = vkbSwapchain.image_format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create render pass!");
	}

	framebuffers.resize(swapchainImageViews.size());
	for(size_t i = 0; i < swapchainImageViews.size(); i++) {
		VkImageView attachments[] = {swapchainImageViews[i]};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = vkbSwapchain.extent.width;
		framebufferInfo.height = vkbSwapchain.extent.height;
		framebufferInfo.layers = 1;

		if(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create framebuffer!");
		}
	}

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

	if(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create command pool!");
	}

	commandBuffers.resize(swapchainImages.size());
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

	if(vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate command buffers!");
	}

	imageAvailableSemaphores.resize(swapchainImages.size());
	renderFinishedSemaphores.resize(swapchainImages.size());
	inFlightFences.resize(swapchainImages.size());

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for(size_t i = 0; i < swapchainImages.size(); i++) {
		if(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
		   vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
		   vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create synchronization objects!");
		}
	}
}

void cleanup_vulkan() {
	vkDeviceWaitIdle(device);

	Render::cleanup();

	for(size_t i = 0; i < swapchainImages.size(); i++) {
		vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(device, inFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(device, commandPool, nullptr);

	for(auto framebuffer : framebuffers) {
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	}

	for(auto imageView : swapchainImageViews) {
		vkDestroyImageView(device, imageView, nullptr);
	}

	vkDestroyRenderPass(device, renderPass, nullptr);

	vmaDestroyAllocator(allocator);
	vkb::destroy_swapchain(vkbSwapchain);
	vkb::destroy_device(vkbDevice);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkb::destroy_instance(vkbInstance);
}

} // namespace Init
