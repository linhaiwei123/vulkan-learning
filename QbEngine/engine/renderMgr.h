#pragma once
#include <string>
#include <unordered_map>
#include <functional>
namespace qb {
	class App;
	class Render;
	class RendeRgr {
	private:
		std::unordered_map<std::string, qb::Render*> _renderMap;
		App* app;
	public:
		void init(App* app);
		void initPbrRender();
		qb::Render* getRender(std::string name);
		void destroy();
	};

	class Render {
	private:
		App* app;
		std::string name;
	public:
		void init(App* app, std::string name);
		std::function<void(size_t i)>* onCmd;
		void destroy();
	};
}