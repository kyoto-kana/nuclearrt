#pragma once

#ifdef NUCLEAR_BACKEND_SDL2

#include "AudioBackend.h"
#include <vector>
#include <unordered_map>
#include <set>
struct stb_vorbis;
struct SoundInfo;

#include <SDL2/SDL.h>

typedef struct SampleFile {
	Uint8 *data = nullptr;
	Uint32 data_len = 0;
	SDL_AudioSpec spec{};
	std::string pathName = "";
} SampleFile;


enum class ChannelStreamType {
	None,
	MemorySample,
	OggFile,
	Mp3File
};

typedef struct Channel {
	Uint8 *data = nullptr; // No need to make floats, as SDL_AudioStream does convert samples into a format you give via SDL_AudioSpec
	Uint32 data_len = 0;
	SDL_AudioSpec spec{};
	bool uninterruptable = false;
	SDL_AudioStream *stream = nullptr;
	int position = 0;
	bool lock = false;
	bool finished = false;
	int curHandle = -1;
	bool loop = false;
	bool pause = false;
	float volume = 1.0f;
	float pan = 0.0f;
	std::string name = "";


	ChannelStreamType streamType = ChannelStreamType::None;
	std::vector<uint8_t> compressedStreamData;



	
	void* decoder = nullptr; // stb_vorbis* or drmp3*
	bool streaming = false;
	bool sourceEnded = false;
	int loopsRemaining = 0;
	int streamSampleRate = 0;
	int streamChannels = 0;
	std::string streamPath;
	std::vector<int16_t> decodeBuffer;
} Channel;

typedef struct CachedSample {
	Uint8* data = nullptr;
	Uint32 data_len = 0;
	SDL_AudioSpec spec{};
	std::string name = "";
} CachedSample;

class SDL2Backend;

class SDL2AudioBackend : public AudioBackend {
public:
	void Initialize() override;
	void Deinitialize() override;

	void SetBackend(SDL2Backend* b) { backend = b; }

	static void SDLCALL AudioCallback(void* userdata, Uint8* stream, int len);
	bool LoadSample(int id, int channel) override;

	bool ShouldStreamSample(int id, SoundInfo* soundInfo, size_t packedSize, int loops);

	bool LoadSampleFromMemory(
		int id,
		int channel,
		SoundInfo* soundInfo,
		const std::vector<uint8_t>& data
	);

	bool LoadStreamingSampleFromMemory(
		int id,
		int channel,
		SoundInfo* soundInfo,
		std::vector<uint8_t>&& data
	);

	bool LoadSampleFile(std::string path) override;
	int FindSample(std::string name) override;
	void PlaySample(int id, int channel, int loops, int freq, bool uninterruptable, float volume, float pan) override;
	void PlaySampleFile(std::string path, int channel, int loops) override;
	void RefillStreamingChannel(Channel& ch);
	void DiscardSampleFile(std::string path) override;
	void UpdateSample() override;
	void PauseSample(int id, bool channel, bool pause) override;
	bool SampleState(int id, bool channel, bool pauseOrStop) override;
	int GetSampleVolume(int id) override;
	int GetSampleVolume(std::string name) override;
	int GetChannelVolume(int id) override;
	std::string GetChannelName(int channel) override;
	void SetSampleVolume(float volume, int id, bool channel) override;
	void LockChannel(int channel, bool unlock) override;
	void SetSamplePan(float pan, int id, bool channel) override;
	int GetSamplePan(int id, bool channel) override;
	void SetSampleFreq(int freq, int id, bool channel) override;
	int GetSampleFreq(int id, bool channel) override;
	int GetSampleDuration(int id, bool channel) override;
	int GetSamplePos(int id, bool channel) override;
	void SetSamplePos(int pos, int id, bool channel) override;
	void StopSample(int id, bool channel) override;
	
	void PreloadSample(int id) override;
	void UnloadPreloadedSample(int id) override;

	SDL_AudioSpec spec;
	SDL2Backend* backend = nullptr;
private:
	static SDL_AudioDeviceID audio_device;
	float mainVol = 100.0f;
	float mainPan = 0.0f;
	Channel channels[49]; // 48 will be the last element.
	std::unordered_map<std::string, SampleFile> sampleFiles;
	std::unordered_map<int, CachedSample> sampleCache;
	bool sampleFocusGainApplied = false;
	bool lastWindowFocused = true;

}; 
#endif