#pragma once
#include <string>
#include <unordered_map>
#include <map>
#include <functional>
#include <any>
namespace qb {
	class App;
	class DynamicData;
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
		std::vector<qb::DynamicData*> _instances;
		std::vector<std::string> _keys;
	public:
		std::vector<std::tuple<std::string, size_t, size_t>> view{};
		std::map<std::pair<std::string, size_t>, size_t> addrTable{};
		size_t size = -1;
	public:
		void init(App* app, std::string name);
		void build();
		size_t getKeySize(std::string key);
		size_t getOffset(std::string key, size_t index = 0);
		const std::vector<std::string>* getKeys();
		qb::DynamicData* getDynamicData();
		void destroy();
	};

	class DynamicData {
	private:
		DynamicStruct* dynamicStruct;
		void* _data;
	public:
		size_t count;
	public:
		void init(DynamicStruct* dynamicStruct);
		void build();
		size_t getTotalSize();
		size_t getUnitSize();
		size_t getOffset(std::string key, size_t index = 0);
		void setData(size_t countIdx, std::string key, size_t index, void* src);
		void setData(std::string key, void*src);
		void setData(std::string key, size_t index, void* src);
		void setData(size_t countIdx, std::string key, void* src);
		void setVertexData(std::vector<std::vector<std::any>> src); // for vertex only now
		void* getMappingAddr();
		void destroy();
	};
}