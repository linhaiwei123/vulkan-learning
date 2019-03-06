#pragma once
#include <chrono>
namespace qb {
	class App;
	class TimerMgr {
	private:
		float _deltaTime;
		float _totalTime;
		std::chrono::time_point<std::chrono::steady_clock> _preTime;
		bool _isInitStartTime = false;
	public:
		App* app;
	public:
		void init(App* app);
		void update();
		float getDeltaTime();
		float getTotalTime();
		void destroy();
	};
}