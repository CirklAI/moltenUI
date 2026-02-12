#include "instance.h"
#include <GLFW/glfw3.h>
#include <string.h>

VkInstance create_instance(void) {
	VkApplicationInfo app_info;
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pNext = NULL;
	char app_name[VK_MAX_EXTENSION_NAME_SIZE];
	strcpy(app_name, "vulkan_project");
	app_info.pApplicationName = app_name;
	app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
	char app_engine_name[VK_MAX_EXTENSION_NAME_SIZE];
	strcpy(app_engine_name, "vulkan_engine");
	app_info.pEngineName = app_engine_name;
	app_info.engineVersion = VK_MAKE_VERSION(0, 0, 1);
	app_info.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo inst_cre_info;
	inst_cre_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	inst_cre_info.pNext = NULL;
	inst_cre_info.flags = 0;
	inst_cre_info.pApplicationInfo = &app_info;
	uint32_t inst_layer_count = 1;
	inst_cre_info.enabledLayerCount = inst_layer_count;
	char pp_inst_layers[inst_layer_count][VK_MAX_EXTENSION_NAME_SIZE];
	strcpy(pp_inst_layers[0], "VK_LAYER_KHRONOS_validation");
	char *pp_inst_layer_names[inst_layer_count];
	for(uint32_t i = 0; i < inst_layer_count; i++) {
		pp_inst_layer_names[i] = pp_inst_layers[i];
	}
	inst_cre_info.ppEnabledLayerNames = (const char *const *)pp_inst_layer_names;
	uint32_t inst_ext_count = 0;
	const char *const *pp_inst_ext_names = glfwGetRequiredInstanceExtensions(&inst_ext_count);
	inst_cre_info.enabledExtensionCount = inst_ext_count;
	inst_cre_info.ppEnabledExtensionNames = pp_inst_ext_names;

	VkInstance inst;
	vkCreateInstance(&inst_cre_info, NULL, &inst);
	return inst;
}
