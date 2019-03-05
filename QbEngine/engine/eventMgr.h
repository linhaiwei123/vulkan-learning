#pragma once
#include <string>
#include <functional>
#include <vector>
#include <memory>
#include <unordered_map>
#include <any>
namespace qb {
	class App;
	class EventEmitter;
	class EventMgr {
	private:
		std::unordered_map<std::string, EventEmitter*> _eventEmitterMap;
	public:
		App* app;
	public:
		void init(App* app);
		EventEmitter* getEventEmitter(std::string name);
		void destroy();
	};
	struct Event;
	class EventEmitter {
	private:
		std::unordered_map<std::string, std::vector<std::function<void(Event& event)>>> _eventOnTable;
		std::unordered_map<std::string, std::vector<std::function<void(Event& event)>>> _eventOnceTable;
	public:
		std::string name;
		App* app;
	public:
		void on(std::string eventName, std::function<void(Event& event)> cb);
		void off(std::string eventName, std::function<void(Event& event)> cb);
		void once(std::string eventName, std::function<void(Event& event)> cb);
		void emit(std::string eventName, Event event);
		void clear();
		void init(App* app, std::string name);
		void destroy();
	};
	struct Event {
		std::any detail;
	};
}