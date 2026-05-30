#pragma once

#include <string>
#include <unordered_set>
#include "Application.h"
#include "PlatformBackend.h"
#include "AppData.h"

class AudioBackend {
public:
	virtual void Initialize() {}
	virtual void Deinitialize() {}

	virtual bool LoadSample(int id, int channel) {return false;}
	virtual bool LoadSampleFile(std::string path) {return false;}
	virtual int FindSample(std::string name) {return -1;}
	virtual void PlaySample(int id, int channel, int loops, int freq, bool uninterruptable, float volume, float pan) {}
	virtual void PlaySampleFile(std::string path, int channel, int loops) {}
	virtual void DiscardSampleFile(std::string path) {}
	virtual void StopSample(int id, bool channel) {}
	virtual void PauseSample(int id, bool channel, bool pause) {}
	virtual void SetSampleVolume(float volume, int id, bool channel) {}
	virtual int GetSampleVolume(int id) {return 0;}
	virtual int GetSampleVolume(std::string name) {return 0;}
	virtual int GetChannelVolume(int id) {return 0;}
	virtual std::string GetChannelName(int channel) {return "";}
	virtual void LockChannel(int channel, bool unlock) {}
	virtual void SetSamplePan(float pan, int id, bool channel) {}
	virtual int GetSamplePan(int id, bool channel) {return 0;}
	virtual int GetSampleFreq(int id, bool channel) {return 0;}
	virtual void SetSampleFreq(int freq, int id, bool channel) {}
	virtual int GetSampleDuration(int id, bool channel) {return 0;}
	virtual int GetSamplePos(int id, bool channel) {return 0;}
	virtual void SetSamplePos(int pos, int id, bool channel) {}
	virtual void UpdateSample() {}
	virtual bool SampleState(int id, bool channel, bool pauseOrStop) {return false;}

	virtual void PreloadSample(int id) {}

	virtual void UnloadPreloadedSample(int id) {}

	virtual void SetFramePreloadedSounds(const int* sounds, int count)
	{
		std::unordered_set<int> nextSounds;

		for (int i = 0; i < count; i++) {
			nextSounds.insert(sounds[i]);
			Application::Instance().GetBackend().get()->platform->Log("Pre-loading sound " + std::to_string(i));
			PreloadSample(sounds[i]);
		}

		if (!Application::Instance().GetAppData()->GetSampleOverFrame()) {
			for (int oldSound : currentFramePreloadedSounds) {
				if (nextSounds.find(oldSound) == nextSounds.end()) {
					UnloadPreloadedSample(oldSound);
				}
			}
		}

		currentFramePreloadedSounds = std::move(nextSounds);
	}

protected:
	std::unordered_set<int> currentFramePreloadedSounds;
}; 