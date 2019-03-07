#include "fontMgr.h"
#include "app.h"
FT_Face qb::FontMgr::getFace(std::string name){
	// read face from cache
	auto it = _faceMap.find(name);
	if (it != _faceMap.end())
		return it->second;
	FT_Face face;
	auto path = get_asset_full_path(name);
	int ftErr;
	ftErr = FT_New_Face(ft, path.c_str(), 0, &face);
	if (ftErr != 0) {
		log_error("Failed to load font: %d", ftErr);
		assert(0);
	}
	_faceMap.insert({ name, face });
	return _faceMap.at(name);
}
qb::Font* qb::FontMgr::getFont(std::string name) {
	// read font from cache
	auto it = _fontMap.find(name);
	if (it != _fontMap.end())
		return it->second;
	qb::Font* font = new qb::Font();
	font->init(app, name);
	_fontMap.insert({ name, font });
	return _fontMap.at(name);
}

void qb::FontMgr::init(App * app){
	this->app = app;
	// free type
	int ftErr;
	ftErr = FT_Init_FreeType(&ft);
	if (ftErr != 0) {
		log_error("Could not init FreeType Library: %d", ftErr);
		assert(0);
	}

	// vertex data
	vertices = {
		{{0.0f, 0.0f}, {0.0f, 0.0f}},
		{{0.0f, 0.0f}, {0.0f, 1.0f}},
		{{0.0f, 0.0f}, {1.0f, 1.0f}},
		{{0.0f, 0.0f}, {1.0f, 0.0f}},
	};

	// index data
	indices = {
		0, 1, 2, 2, 3, 0
	};

	// vertex buf
	vertexBuf = app->bufferMgr.getBuffer("$vertex_FontMgr");
	vertexBuf->bufferInfo.size = sizeof(qb::FontMgr::Vertex) * vertices.size();
	vertexBuf->bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	vertexBuf->build();
	vertexBuf->mapping(vertices.data());

	// index buf
	indexBuf = app->bufferMgr.getBuffer("$index_FontMgr");
	indexBuf->bufferInfo.size = sizeof(uint16_t) * indices.size();
	indexBuf->bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	indexBuf->build();
	indexBuf->mapping(indices.data());

	// descriptor desc
	descriptorDesc = app->descriptorMgr.getDescriptorDesc("$fontDescriptor");
	descriptorDesc->bindings = {
		descriptor_layout_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT),
		descriptor_layout_binding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT),
		descriptor_layout_binding(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT),
		descriptor_layout_binding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT),
	};
	descriptorDesc->build();
}

void qb::FontMgr::destroy(){
	for (auto it : _faceMap) {
		FT_Done_Face(it.second);
	}
	for (auto it : _fontMap) {
		it.second->destroy();
		delete it.second;
	}
	FT_Done_FreeType(ft);
}

void qb::Font::init(App * app, std::string name){
	this->app = app;
	this->name = name;
	this->font_size = 48u;
}

void qb::Font::build(){
	assert(face != nullptr);
	//FT_Set_Char_Size(face, 0, 16 * 64, app->swapchain.extent.width, app->swapchain.extent.height);
	FT_Set_Pixel_Sizes(face, 0, this->font_size);
}

qb::Text* qb::Font::getText(std::string content){
	// read text from cache
	auto it = _textMap.find(name);
	if (it != _textMap.end())
		return it->second;
	qb::Text* text = new qb::Text();
	text->init(app, content);
	_textMap.insert({ name, text });

	std::vector<qb::Font::Character*> characters;
	for (wchar_t ch : content) {
		auto it = _characterMap.find(ch);
		if (it != _characterMap.end()) {
			characters.push_back(it->second);
			continue;
		}
		int ftErr;
		ftErr = FT_Load_Char(face, ch, FT_LOAD_RENDER);
		if (ftErr != 0) {
			log_error("Failed to load Glyph: %d", ftErr);
			assert(0);
		}

		// character img
		std::wstring chWStr{ch};
		std::string chStr{chWStr.begin(), chWStr.end()};
		auto fontName = "$font_" + this->name + "/" + chStr;
		qb::Image* img = app->bufferMgr.getImage("$tex_" + fontName);
		img->imageInfo.imageType = VK_IMAGE_TYPE_2D;
		img->imageInfo.format = VK_FORMAT_R8_UINT;
		img->imageInfo.extent = {
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			1,
		};
		img->imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		img->viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		img->viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		img->descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		img->build();
		size_t size = face->glyph->bitmap.width * face->glyph->bitmap.rows;
		img->mapping(face->glyph->bitmap.buffer, size);
		
		img->setImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		
		// character uniform
		qb::Buffer* uniBuf = app->bufferMgr.getBuffer("$characterUniform_" + fontName);
		uniBuf->bufferInfo.size = sizeof(qb::Font::Character::Uniform);
		uniBuf->bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		uniBuf->descriptorRange = sizeof(qb::Font::Character::Uniform);
		uniBuf->buildPerSwapchainImg();

		// character
		Character* character = new Character{
			ch,
			chStr,
			img,
			uniBuf,
			{
				glm::vec2((float)face->glyph->bitmap.width/ (float)font_size, (float)face->glyph->bitmap.rows/ (float)font_size),
				glm::vec2((float)face->glyph->bitmap_left / (float)font_size, (float)face->glyph->bitmap_top / (float)font_size),
			},
			glm::vec2((float)(face->glyph->advance.x>>6) / (float)font_size,(float)(face->glyph->advance.y>>6) / (float)font_size)
		};
		_characterMap.insert({ ch, character });
		characters.push_back(character);
	}
	
	text->characters = characters;
	text->build();

	return text;
}

void qb::Font::destroy(){
	for (auto& it : _characterMap) {
		it.second->img = nullptr;
		delete it.second;
	}

	for (auto& it : _textMap) {
		it.second->destroy();
		delete it.second;
	}
	face = nullptr;
}

void qb::Text::init(App* app, std::string content){
	this->app = app;
	this->content = content;
	this->contentHash = std::hash<std::string>{}(this->content);
}

void qb::Text::build(){
	std::string uniqueName = "$text_" + std::to_string(this->contentHash);
	// mvp uniform
	mvpUniBuf = app->bufferMgr.getBuffer("$mvpUniform_" + uniqueName);
	mvpUniBuf->bufferInfo.size = sizeof(qb::Text::Uniform);
	mvpUniBuf->bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	mvpUniBuf->descriptorRange = sizeof(qb::Text::Uniform);
	mvpUniBuf->buildPerSwapchainImg();
	// character advance uniform/ descriptor
	size_t chIndex = 0;
	for (auto& character : characters) {
		// uniform
		std::string uniformName = "$uniform_" + uniqueName + "/" + character->contentStr + "/" + std::to_string(chIndex);
		qb::Buffer*	uniBuf = app->bufferMgr.getBuffer(uniformName);
		uniBuf->bufferInfo.size = sizeof(qb::Text::AdvanceUniform);
		uniBuf->bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		uniBuf->descriptorRange = sizeof(qb::Text::AdvanceUniform);
		uniBuf->buildPerSwapchainImg();
		this->uniBufs.push_back(uniBuf);
		// descriptor
		std::string descriptorName = "$descriptor_" + uniqueName + "/" + character->contentStr + "/" + std::to_string(chIndex);
		qb::Descriptor* descriptor = app->descriptorMgr.getDescriptor(descriptorName);
		descriptor->desc = app->fontMgr.descriptorDesc;
		descriptor->datas = {
			mvpUniBuf,
			character->uniBuf,
			uniBuf,
			character->img,
		};
		descriptor->buildPerSwapchainImg();
		this->descriptors.push_back(descriptor);
		chIndex++;
	}
}

void qb::Text::destroy(){
	for (auto& it : characters)
		it = nullptr;
	for (auto& it : uniBufs)
		it = nullptr;
	for (auto& it : descriptors)
		it = nullptr;
	this->mvpUniBuf = nullptr;
}
