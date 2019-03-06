#pragma once
#include <string>
#include <unordered_map>
#include <map>
#include <functional>
namespace qb {
	class App;
	class DynamicStructInstance;
	class DynamicStruct;
	class DynamicStructMgr {
	private:
		std::unordered_map<std::string, qb::DynamicStruct*> _dynamicStructMap;
		App* app;
	public:
		void init(App* app);
		qb::DynamicStruct* getDynamicStruct(std::string name);
		void destroy();
	};

	class DynamicStruct {
	private:
		App* app;
		std::string name;
		std::vector<qb::DynamicStructInstance*> _instances;
	public:
		std::vector<std::tuple<std::string, size_t, size_t>> view{};
		std::map<std::pair<std::string, size_t>, size_t> addrTable{};
		size_t size = -1;
	public:
		void init(App* app, std::string name);
		void build();
		size_t getKeySize(std::string key);
		qb::DynamicStructInstance* create();
		void destroy();
	};

	class DynamicStructInstance {
	private:
		DynamicStruct* dynamicStruct;
	public:
		void* data;
	public:
		void init(DynamicStruct* dynamicStruct);
		size_t getSize();
		size_t getOffset(std::string key, size_t index = 0);
		void setData(std::string key, size_t index, void* src);
		void destroy();
	};
}