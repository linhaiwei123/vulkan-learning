#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <exception>
#include <iostream>
#include <functional>
#include <assert.h>
#include <algorithm>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "tool.h"
namespace qb {
	class App;
	class Image;
	class Buffer;
	class Descriptor; 
	class Text;
	class Font;
	class FontMgr {
	private:
		std::unordered_map<std::string, FT_Face> _faceMap;
		std::unordered_map<std::string, qb::Font*> _fontMap;
	public:
		FT_Library ft;
		App *app;
		struct Vertex {
			glm::vec2 pos;
			glm::vec2 texCoord;
			vertex_desc(qb::FontMgr::Vertex, 0, VK_VERTEX_INPUT_RATE_VERTEX,
				{ 0, VK_FORMAT_R32G32_SFLOAT, offsetof(qb::FontMgr::Vertex, pos) },
				{ 1, VK_FORMAT_R32G32_SFLOAT, offsetof(qb::FontMgr::Vertex, texCoord) },
			);
		};
		Buffer* vertexBuf;
		Buffer* indexBuf;
		std::vector<uint16_t> indices{};
		std::vector <qb::FontMgr::Vertex> vertices{};
	public:
		FontMgr() = default;

		FT_Face getFace(std::string name);

		qb::Font* getFont(std::string name);

		void init(App *app);

		void destroy();
	};

	class Font {
	public:
		struct Character {
			wchar_t content;
			std::string contentStr;
			qb::Image* img;
			qb::Buffer* uniBuf;
			struct Uniform {
				glm::vec2 size;
				glm::vec2 bearing;
			}uniform;
			glm::vec2 advance;
		};
		uint32_t font_size;
	private:
		std::unordered_map<wchar_t, Character*> _characterMap;
		std::unordered_map<std::string, qb::Text*> _textMap;
		qb::App* app;
	public:
		std::string name;
	public: // free type
		FT_Face face;
	public:
		Font() = default;

		void init(App* app, std::string name);
		void build();
		qb::Text* getText(std::string content);
		void destroy();
	};

	class Text {
	private:
		std::string content;
		std::size_t contentHash;
	public:
		struct Uniform {
			glm::mat4 model;
			glm::mat4 view;
			glm::mat4 project;
		}uniform;
		struct AdvanceUniform {
			glm::vec2 advance;
		} advUniform;
		std::vector<qb::Font::Character*> characters;
		std::vector<qb::Buffer*> uniBufs;
		std::vector<qb::Descriptor*> descriptors;
		qb::Buffer* mvpUniBuf;
		App* app;
	public:
		Text() = default;
		void init(App* app, std::string content);
		void build();
		void destroy();
	};
};