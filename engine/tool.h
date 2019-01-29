#pragma once

//! log
// log bg color black = 40,red = 41,green = 42,yellow = 43,blue = 44,purple = 45,dark_green = 46,white = 47
// log font color black = 30,red = 31,green = 32,yellow = 33,blue = 34,purple = 35,dark_green = 36,white = 37
#define log_info(format, ...) {fprintf(stdout,"\033[40;32m" format "\033[40;37m" "\n", ##__VA_ARGS__);}
#define log_error(format, ...) {fprintf(stdout,"\033[40;31m" format "\033[40;37m" "\n", ##__VA_ARGS__);}
#define log_warning(format, ...) {fprintf(stdout,"\033[40;33m" format "\033[40;37m" "\n", ##__VA_ARGS__);}

//! vk_check
#define vk_check(v)	do {\
	if (v != VK_SUCCESS) {\
		log_error("vk_check failed:\n error %d\n file: %s\n line: %d\n func: %s\n code: %s\n", v, __FILE__, __LINE__,__FUNCTION__, #v);\
		assert(v == VK_SUCCESS);\
	}\
} while(0)

//! vertex desc
#define vertex_binding_desc(binding, stride, inputRate)\
static VkVertexInputBindingDescription getBindingDesc() {\
	return {binding, stride, inputRate};\
}

#define vertex_attrib_desc(...)\
static std::vector<VkVertexInputAttributeDescription> getAttribDesc() {\
	std::vector<VkVertexInputAttributeDescription> attrib { __VA_ARGS__ } ;\
	return attrib;\
}

//! uniform layout
#define descriptor_layout_binding(binding,descriptorType,stageFlags)\
static VkDescriptorSetLayoutBinding getLayoutBinding() {\
	return {binding,descriptorType,1,stageFlags,nullptr};\
}

//! type
#define is_type(instance,type) (typeid(instance).hash_code() == typeid(type).hash_code())

