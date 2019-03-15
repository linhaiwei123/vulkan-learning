#pragma once
#include "json.hpp"
#include <unordered_map>
namespace qb {
	class App;
	class FileMgr {
	private:
		App* app;
		std::unordered_map<std::string, nlohmann::json*> _jsonMap;
	public:
		void init(App* app);
		nlohmann::json* getJson(std::string name);
		void destroy();
	};
}