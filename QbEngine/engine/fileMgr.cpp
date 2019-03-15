#include "fileMgr.h"
#include "tool.h"
#include <fstream>
void qb::FileMgr::init(App * app) {
	this->app = app;
}

nlohmann::json * qb::FileMgr::getJson(std::string name) {
	// read json from cache
	auto it = _jsonMap.find(name);
	if (it != _jsonMap.end())
		return it->second;
	auto path = get_asset_full_path(name);
	std::ifstream file(path, std::ifstream::binary);
	nlohmann::json* json = new nlohmann::json(nlohmann::json::parse(file));
	file.close();
	_jsonMap.insert({ name, json });
	return _jsonMap.at(name);
}

void qb::FileMgr::destroy() {
	for (auto& it : _jsonMap)
		delete it.second;
}
