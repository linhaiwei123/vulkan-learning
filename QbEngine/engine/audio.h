#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <exception>
#include <iostream>
#include <functional>
#include <assert.h>
#include <algorithm>
#include "fmod_studio.hpp"
#include "fmod.hpp"
#include "fmod_errors.h"
namespace qb {
	class App;
	class AudioClip;
	class AudioMgr {
	private:
		std::unordered_map<std::string, FMOD::Sound*> _soundMap;
		std::unordered_map<std::string, qb::AudioClip*> _audioClipMap;
	public:
		FMOD::Studio::System* system = nullptr;
		FMOD::System* lowLevelSystem = nullptr;
		App *app;
	public:
		AudioMgr() = default;

		void init(App *app);

		AudioClip* getAudioClip(std::string name);

		FMOD::Sound* getSound(std::string name);

		void update();

		void destroy();
	};

	class AudioClip {
	private:
		std::string name;
		App* app;
		FMOD::Channel* channel;
	public:
		FMOD::Sound* sound;
		void init(App* app, std::string name);

		void build();

		void play(bool isLoop=false);

		void destroy();
	};
};