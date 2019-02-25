#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
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
#define vertex_desc(T, binding, inputRate, ...)\
static VkVertexInputBindingDescription getBindingDesc() {\
	return {binding, sizeof(T), inputRate};\
}\
static uint32_t getBinding() { return binding; }\
static std::vector<VkVertexInputAttributeDescription> getAttribDesc() {\
		std::vector <std::tuple<uint32_t, VkFormat, size_t>> attribTemps = {__VA_ARGS__};\
		std::vector<VkVertexInputAttributeDescription> attribDesc = {};\
		for(auto& attrib : attribTemps) {\
			auto& [location, format, offset] = attrib;\
			attribDesc.push_back({location, binding, format, static_cast<uint32_t>(offset)});\
		}\
		return attribDesc;\
}

//! uniform layout
#define descriptor_layout_binding(binding,descriptorType,stageFlags) {binding,descriptorType,1,stageFlags,nullptr}

//! vertex binding (0-31)
#define model_vertex_binding_id 31

//! model 
#define max_bones_per_mesh 64
#define max_bones_per_vertex 4

//! asset (temp)
#define asset_base_path "F:/Vulkan/projects/res/"
#define get_asset_full_path(name) asset_base_path + name
