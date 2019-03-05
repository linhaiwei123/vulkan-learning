#include "audioMgr.h"
#include "app.h"

void qb::AudioMgr::init(App * app){
	this->app = app;
	// fmod studio
	FMOD_RESULT result;
	result = FMOD::Studio::System::create(&system);
	if (result != FMOD_OK) {
		log_error("failed to create fmod studio system: %d", result);
		assert(0);
	}
	result = system->getLowLevelSystem(&lowLevelSystem);
	if (result != FMOD_OK) {
		log_error("failed to create fmod low level system: %d", result);
		assert(0);
	}
	system->initialize(1024, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, nullptr);
}

qb::AudioClip * qb::AudioMgr::getAudioClip(std::string name){
	// read audio clip from cache
	auto it = _audioClipMap.find(name);
	if (it != _audioClipMap.end())
		return it->second;
	qb::AudioClip* audioClip = new qb::AudioClip();
	audioClip->init(app, name);
	_audioClipMap.insert({ name, audioClip });
	return _audioClipMap.at(name);
}

FMOD::Sound * qb::AudioMgr::getSound(std::string name){
	// read sound from cache
	auto it = _soundMap.find(name);
	if (it != _soundMap.end())
		return it->second;
	FMOD::Sound* sound;
	auto path = get_asset_full_path(name);
	FMOD_RESULT result = lowLevelSystem->createSound(path.c_str(), FMOD_DEFAULT, nullptr, &sound);
	if (result != FMOD_OK) {
		log_error("failed to create sound: %s", name.c_str());
		assert(0);
	}
	_soundMap.insert({ name, sound });
	return _soundMap.at(name);
}

void qb::AudioMgr::update(){
	FMOD_RESULT result = system->update();
	if (result != FMOD_OK) {
		log_error("fmod studio update failed: %d", result);
		assert(0);
	}
}

void qb::AudioMgr::destroy(){
	// sound
	for (auto& it : _soundMap)
		it.second->release();
	for (auto& it : _audioClipMap) {
		it.second->destroy();
		delete it.second;
	}

	// system
	FMOD_RESULT result = this->system->release();
	if (result != FMOD_OK) {
		log_error("failed to release fmod studio system: %d", result);
		assert(0);
	}
	this->lowLevelSystem = nullptr;
}

void qb::AudioClip::init(App * app, std::string name){
	this->app = app;
	this->name = name;
}

void qb::AudioClip::build(){
	assert(this->sound != nullptr);
}

void qb::AudioClip::play(bool isLoop){
	FMOD_RESULT result = app->audioMgr.lowLevelSystem->playSound(this->sound, nullptr, false, &channel);
	if (result != FMOD_OK) {
		log_error("failed to play sound: %d", result);
		assert(0);
	}

	if (!isLoop)
		channel->setMode(FMOD_LOOP_OFF);
	else
	{
		channel->setMode(FMOD_LOOP_NORMAL);
		channel->setLoopCount(-1);
	}
}

void qb::AudioClip::destroy(){
	this->sound = nullptr;
	this->channel = nullptr;
}
