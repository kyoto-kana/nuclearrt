#ifdef NUCLEAR_BACKEND_SDL2

#include "SDL2AudioBackend.h"

#include "Application.h"
#include "SoundBank.h"
#include "SDL2Backend.h"

#include <SDL2/SDL.h>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <iostream>
#include "./libs/stb_vorbis.c"

#define DR_MP3_IMPLEMENTATION
#include "./libs/dr_mp3.h"


class AudioDeviceLock {
public:
	explicit AudioDeviceLock(SDL_AudioDeviceID dev) : device(dev) {
		if (device != 0) {
			SDL_LockAudioDevice(device);
			locked = true;
		}
	}

	~AudioDeviceLock() { Unlock(); }

	void Unlock() {
		if (locked && device != 0) {
			SDL_UnlockAudioDevice(device);
			locked = false;
		}
	}

	AudioDeviceLock(const AudioDeviceLock&) = delete;
	AudioDeviceLock& operator=(const AudioDeviceLock&) = delete;

private:
	SDL_AudioDeviceID device = 0;
	bool locked = false;
};

SDL_AudioDeviceID SDL2AudioBackend::audio_device = 0;

void SDLCALL SDL2AudioBackend::AudioCallback(void* userdata, Uint8* stream, int len)
{
	auto* self = static_cast<SDL2AudioBackend*>(userdata);
	SDL_memset(stream, 0, len);
	if (!self) return;

	float* out = reinterpret_cast<float*>(stream);
	const int frames = len / (sizeof(float) * 2);

	for (int i = 0; i < frames; ++i) {
		float left = 0.0f;
		float right = 0.0f;

		for (int ch = 1; ch < static_cast<int>(SDL_arraysize(self->channels)); ++ch) {
			Channel& channel = self->channels[ch];

			if (!channel.stream || channel.pause)
				continue;

			float tempData[2] = {0.0f, 0.0f};
			const int got = SDL_AudioStreamGet(channel.stream, tempData, static_cast<int>(sizeof(tempData)));

			if (got <= 0) {
				channel.finished = true;
				continue;
			}

			channel.position += got / static_cast<int>(sizeof(float) * 2);

			const float pi = 3.14159265358979323846f;
			const float angle = (channel.pan + 1.0f) * 0.25f * pi;
			const float leftGain = cosf(angle) * channel.volume;
			const float rightGain = sinf(angle) * channel.volume;

			left += tempData[0] * leftGain;
			right += tempData[1] * rightGain;
		}

		out[i * 2 + 0] = SDL_clamp(left, -1.0f, 1.0f);
		out[i * 2 + 1] = SDL_clamp(right, -1.0f, 1.0f);
	}
}


void SDL2AudioBackend::Initialize()
{
	SDL_AudioSpec want{};
	want.freq = 44100;
	want.channels = 2;
	want.format = AUDIO_F32SYS;
	want.samples = 4096;
	want.callback = SDL2AudioBackend::AudioCallback;
	want.userdata = this;

	audio_device = SDL_OpenAudioDevice(nullptr, 0, &want, &spec, 0);

	if (!audio_device) {
		backend->GetPlatform()->Log("SDL_OpenAudioDevice Error : " + std::string(SDL_GetError()));
		return;
	}

	SDL_PauseAudioDevice(audio_device, 0);
	backend->GetPlatform()->Log("Opened Audio Device.");
}

void SDL2AudioBackend::Deinitialize()
{
	{
		AudioDeviceLock lock(audio_device);

		while (!sampleFiles.empty()) {
			auto it = sampleFiles.begin();
			if (it->second.data) SDL_free(it->second.data);
			sampleFiles.erase(it);
		}

		for (int i = 1; i < static_cast<int>(SDL_arraysize(channels)); ++i) {
			Channel& ch = channels[i];
			if (ch.stream) {
				SDL_AudioStreamClear(ch.stream);
				SDL_FreeAudioStream(ch.stream);
				ch.stream = nullptr;
			}
			if (ch.data) {
				SDL_free(ch.data);
				ch.data = nullptr;
			}
			ch.data_len = 0;
			ch.curHandle = -1;
			ch.finished = false;
			ch.loop = false;
			ch.pause = false;
			ch.lock = false;
			ch.uninterruptable = false;
			ch.position = 0;
			ch.name.clear();
		}
	}

	for (auto& [id, sample] : sampleCache) {
		if (sample.data) {
			SDL_free(sample.data);
			sample.data = nullptr;
		}
	}
	sampleCache.clear();

	if (audio_device) {
		SDL_PauseAudioDevice(audio_device, 1);
		SDL_CloseAudioDevice(audio_device);
		audio_device = 0;
	}

	backend->GetPlatform()->Log("AudioBackend shut down successfully.");
}

bool SDL2AudioBackend::LoadSample(int id, int channel)
{
	SoundInfo* soundInfo = SoundBank::Instance().GetSound(id);

	if (!soundInfo) {
		backend->GetPlatform()->Log(
			"SoundBank Error: Sound ID " + std::to_string(id) + " not found!"
		);
		return false;
	}

	std::vector<uint8_t> data =
		backend->platform->GetPakFile().GetData(
			"sounds/" + std::to_string(id) + "." + soundInfo->Type
		);

	if (data.empty()) {
		backend->GetPlatform()->Log(
			"PakFile::GetData Error: Sample with id "
			+ std::to_string(id) + " not found"
		);
		return false;
	}

	Channel& ch = channels[channel];

	const bool shouldStream =
		ShouldStreamSample(
			id,
			soundInfo,
			data.size(),
			ch.loop ? -1 : 1
		);

	if (shouldStream) {
		return LoadStreamingSampleFromMemory(
			id,
			channel,
			soundInfo,
			std::move(data)
		);
	}

	return LoadSampleFromMemory(
		id,
		channel,
		soundInfo,
		data
	);
}

static bool CopyCachedSampleToChannel(const CachedSample& sample, Channel& ch)
{
	if (ch.data) {
		SDL_free(ch.data);
		ch.data = nullptr;
		ch.data_len = 0;
	}

	ch.data = static_cast<Uint8*>(SDL_malloc(sample.data_len));
	if (!ch.data)
		return false;

	SDL_memcpy(ch.data, sample.data, sample.data_len);
	ch.data_len = sample.data_len;
	ch.spec = sample.spec;
	ch.name = sample.name;
	ch.streaming = false;

	return true;
}

bool SDL2AudioBackend::LoadSampleFromMemory(
	int id,
	int channel,
	SoundInfo* soundInfo,
	const std::vector<uint8_t>& data
)
{
	Channel& ch = channels[channel];

	if (ch.data) {
		SDL_free(ch.data);
		ch.data = nullptr;
		ch.data_len = 0;
	}

	if (soundInfo->Type == "wav") {
		SDL_RWops* rw = SDL_RWFromConstMem(data.data(), static_cast<int>(data.size()));
		if (!rw)
			return false;

		if (!SDL_LoadWAV_RW(rw, 1, &ch.spec, &ch.data, &ch.data_len))
			return false;
	}
	else if (soundInfo->Type == "ogg") {
		int decodedChannels = 0;
		int samplerate = 0;
		short* output = nullptr;

		const int numSamples = stb_vorbis_decode_memory(
			data.data(),
			static_cast<int>(data.size()),
			&decodedChannels,
			&samplerate,
			&output
		);

		if (numSamples <= 0 || !output)
			return false;

		const int totalSamples = numSamples * decodedChannels;
		ch.data_len = totalSamples * sizeof(short);
		ch.data = static_cast<Uint8*>(SDL_malloc(ch.data_len));

		if (!ch.data) {
			free(output);
			return false;
		}

		SDL_memcpy(ch.data, output, ch.data_len);

		ch.spec.freq = samplerate;
		ch.spec.channels = static_cast<Uint8>(decodedChannels);
		ch.spec.format = AUDIO_S16SYS;

		free(output);
	}
	else if (soundInfo->Type == "mp3") {
		drmp3 mp3;

		if (!drmp3_init_memory(&mp3, data.data(), data.size(), nullptr))
			return false;

		const drmp3_uint64 frameCount = drmp3_get_pcm_frame_count(&mp3);
		if (frameCount == 0) {
			drmp3_uninit(&mp3);
			return false;
		}

		const drmp3_uint64 totalSamples64 = frameCount * mp3.channels;
		ch.data_len = static_cast<Uint32>(totalSamples64 * sizeof(int16_t));
		ch.data = static_cast<Uint8*>(SDL_malloc(ch.data_len));

		if (!ch.data) {
			drmp3_uninit(&mp3);
			return false;
		}

		drmp3_read_pcm_frames_s16(&mp3, frameCount, reinterpret_cast<drmp3_int16*>(ch.data));

		ch.spec.channels = static_cast<Uint8>(mp3.channels);
		ch.spec.format = AUDIO_S16SYS;
		ch.spec.freq = static_cast<int>(mp3.sampleRate);

		drmp3_uninit(&mp3);
	}
	else {
		return false;
	}

	ch.name = soundInfo->Name;
	ch.streaming = false;

	ch.stream = SDL_NewAudioStream(
		ch.spec.format,
		ch.spec.channels,
		ch.spec.freq,
		spec.format,
		spec.channels,
		spec.freq
	);

	return ch.stream != nullptr;
}


bool SDL2AudioBackend::LoadStreamingSampleFromMemory(
	int id,
	int channel,
	SoundInfo* soundInfo,
	std::vector<uint8_t>&& data
)
{
	Channel& ch = channels[channel];

	ch.compressedStreamData = std::move(data);
	ch.streaming = true;
	ch.sourceEnded = false;
	ch.curHandle = id;
	ch.name = soundInfo->Name;
	ch.data = nullptr;
	ch.data_len = 0;

	if (soundInfo->Type == "ogg") {
		int error = 0;

		stb_vorbis* ogg = stb_vorbis_open_memory(
			ch.compressedStreamData.data(),
			static_cast<int>(ch.compressedStreamData.size()),
			&error,
			nullptr
		);

		if (!ogg) {
			ch.compressedStreamData.clear();
			return false;
		}

		stb_vorbis_info info = stb_vorbis_get_info(ogg);

		ch.decoder = ogg;
		ch.spec.freq = info.sample_rate;
		ch.spec.channels = static_cast<Uint8>(info.channels);
		ch.spec.format = AUDIO_S16SYS;
		ch.streamType = ChannelStreamType::OggFile;
	}
	else if (soundInfo->Type == "mp3") {
		drmp3* mp3 = static_cast<drmp3*>(SDL_calloc(1, sizeof(drmp3)));

		if (!mp3 || !drmp3_init_memory(mp3, ch.compressedStreamData.data(), ch.compressedStreamData.size(), nullptr)) {
			if (mp3)
				SDL_free(mp3);

			ch.compressedStreamData.clear();
			return false;
		}

		ch.decoder = mp3;
		ch.spec.freq = static_cast<int>(mp3->sampleRate);
		ch.spec.channels = static_cast<Uint8>(mp3->channels);
		ch.spec.format = AUDIO_S16SYS;
		ch.streamType = ChannelStreamType::Mp3File;
	}
	else {
		ch.compressedStreamData.clear();
		return false;
	}

	ch.stream = SDL_NewAudioStream(
		ch.spec.format,
		ch.spec.channels,
		ch.spec.freq,
		spec.format,
		spec.channels,
		spec.freq
	);

	if (!ch.stream) {
		StopSample(channel, true);
		return false;
	}

	ch.decodeBuffer.resize(8192 * ch.spec.channels);
	return true;
}

bool SDL2AudioBackend::LoadSampleFile(std::string path)
{
	AudioDeviceLock lock(audio_device);
	backend->GetPlatform()->Log("Loading Sample File : " + path);

	std::filesystem::path fullPath = path;
	std::string type = fullPath.extension().string();
	SampleFile sampleFile;

	if (type == ".wav") {
		if (!SDL_LoadWAV(path.c_str(), &sampleFile.spec, &sampleFile.data, &sampleFile.data_len)) {
			backend->GetPlatform()->Log("Failed to load WAV file : " + std::string(SDL_GetError()));
			return false;
		}
		backend->GetPlatform()->Log("Loaded WAV Sample File : " + path);
	}
	else if (type == ".ogg") {
		int decodedChannels = 0;
		int samplerate = 0;
		short* output = nullptr;
		const int numSamples = stb_vorbis_decode_filename(path.c_str(), &decodedChannels, &samplerate, &output);
		if (numSamples <= 0 || !output) {
			backend->GetPlatform()->Log("Failed to load OGG file : " + path);
			return false;
		}

		const int totalSamples = numSamples * decodedChannels;
		sampleFile.data_len = totalSamples * sizeof(short);
		sampleFile.data = static_cast<Uint8*>(SDL_malloc(sampleFile.data_len));
		if (!sampleFile.data) {
			free(output);
			backend->GetPlatform()->Log("SDL_malloc failed for OGG file.");
			return false;
		}

		SDL_memcpy(sampleFile.data, output, sampleFile.data_len);
		sampleFile.spec.freq = samplerate;
		sampleFile.spec.format = AUDIO_S16SYS;
		sampleFile.spec.channels = static_cast<Uint8>(decodedChannels);
		free(output);
		backend->GetPlatform()->Log("Loaded OGG file : " + path);
	}
	else if (type == ".mp3") {
		drmp3 mp3;
		if (!drmp3_init_file(&mp3, path.c_str(), nullptr)) {
			backend->platform->Log("Couldn't find MP3 file.");
			return false;
		}

		const drmp3_uint64 frameCount = drmp3_get_pcm_frame_count(&mp3);
		if (frameCount == 0) {
			backend->GetPlatform()->Log("No sample frames in MP3.");
			drmp3_uninit(&mp3);
			return false;
		}

		const drmp3_uint64 totalSamples64 = frameCount * mp3.channels;
		const Uint32 dataLen = static_cast<Uint32>(totalSamples64 * sizeof(int16_t));
		sampleFile.data = static_cast<Uint8*>(SDL_malloc(dataLen));
		if (!sampleFile.data) {
			backend->platform->Log("Bad MP3 Data.");
			drmp3_uninit(&mp3);
			return false;
		}

		drmp3_read_pcm_frames_s16(&mp3, frameCount, reinterpret_cast<drmp3_int16*>(sampleFile.data));
		sampleFile.data_len = dataLen;
		sampleFile.spec.channels = static_cast<Uint8>(mp3.channels);
		sampleFile.spec.format = AUDIO_S16SYS;
		sampleFile.spec.freq = static_cast<int>(mp3.sampleRate);
		drmp3_uninit(&mp3);
	}
	else {
		backend->GetPlatform()->Log("Audio File " + type + " not supported.");
		return false;
	}

	sampleFile.pathName = path;
	sampleFiles.emplace(sampleFile.pathName, sampleFile);
	return true;
}

int SDL2AudioBackend::FindSample(std::string name)
{
	SoundInfo* soundInfo = SoundBank::Instance().GetSoundName(name);
	return soundInfo ? soundInfo->Handle : -1;
}

bool SDL2AudioBackend::ShouldStreamSample(int id, SoundInfo* soundInfo, size_t packedSize, int loops)
{
	(void)id;

	if (!soundInfo)
		return false;

	if (soundInfo->Type == "ogg" || soundInfo->Type == "mp3") {
		if (loops <= 0)
			return true;

		return packedSize >= 512 * 1024;
	}
/*
	if (soundInfo->Type == "wav") {
		return packedSize >= 1024 * 1024;
	}
*/
	return false;
}

void SDL2AudioBackend::PlaySample(int id, int channel, int loops, int freq, bool uninterruptable, float volume, float pan)
{
	AudioDeviceLock lock(audio_device);

	if (channel < 1 || channel >= static_cast<int>(SDL_arraysize(channels))) {
		bool channelFound = false;

		for (int i = 1; i < static_cast<int>(SDL_arraysize(channels)); ++i) {
			if (!channels[i].stream && !channels[i].data && !channels[i].lock) {
				channel = i;
				channelFound = true;
				break;
			}
		}

		if (!channelFound)
			channel = 48;
	}
	else if (channels[channel].stream || channels[channel].data) {
		if (channels[channel].uninterruptable && !uninterruptable)
			return;
	}

	StopSample(channel, true);

	Channel& ch = channels[channel];

	ch.loop = loops <= 0;
	ch.loopsRemaining = ch.loop ? -1 : loops;

	auto cached = sampleCache.find(id);
	if (cached != sampleCache.end()) {
		if (!CopyCachedSampleToChannel(cached->second, ch))
			return;

		ch.stream = SDL_NewAudioStream(
			ch.spec.format,
			ch.spec.channels,
			ch.spec.freq,
			spec.format,
			spec.channels,
			spec.freq
		);

		if (!ch.stream)
			return;
	}
	else {
		if (!LoadSample(id, channel))
			return;
	}

	ch.position = 0;
	ch.pause = false;
	ch.finished = false;
	ch.uninterruptable = uninterruptable;
	ch.curHandle = id;

	if (volume > -1.0f)
		ch.volume = SDL_clamp(volume / 100.0f, 0.0f, 1.0f);

	if (pan != -2.0f)
		ch.pan = SDL_clamp(pan / 100.0f, -1.0f, 1.0f);

	if (!ch.streaming) {
		if (ch.loop) {
			SDL_AudioStreamPut(ch.stream, ch.data, ch.data_len);
		}
		else {
			for (int i = 1; i <= loops; ++i)
				SDL_AudioStreamPut(ch.stream, ch.data, ch.data_len);
		}
	}
	else {
		ch.loop = loops <= 0;
		ch.loopsRemaining = ch.loop ? -1 : loops;

		for (int i = 0; i < 4; ++i)
			RefillStreamingChannel(ch);
	}

	if (freq > 0)
		SetSampleFreq(freq, channel, true);
}

void SDL2AudioBackend::PlaySampleFile(std::string path, int channel, int loops)
{
	AudioDeviceLock lock(audio_device);

	if (channel < 1 || channel >= static_cast<int>(SDL_arraysize(channels)))
		return;

	StopSample(channel, true);

	Channel& ch = channels[channel];

	ch.streamPath = path;
	ch.streaming = true;
	ch.sourceEnded = false;
	ch.loop = loops <= 0;
	ch.loopsRemaining = ch.loop ? -1 : loops;
	ch.position = 0;
	ch.pause = false;
	ch.finished = false;
	ch.name = path;
	ch.volume = 1.0f;
	ch.pan = 0.0f;

	std::filesystem::path fullPath = path;
	std::string ext = fullPath.extension().string();

	if (ext == ".ogg") {
		int error = 0;
		stb_vorbis* ogg = stb_vorbis_open_filename(path.c_str(), &error, nullptr);

		if (!ogg) {
			backend->GetPlatform()->Log("Failed to stream OGG: " + path);
			StopSample(channel, true);
			return;
		}

		stb_vorbis_info info = stb_vorbis_get_info(ogg);

		ch.decoder = ogg;
		ch.streamSampleRate = info.sample_rate;
		ch.streamChannels = info.channels;
		ch.spec.freq = info.sample_rate;
		ch.spec.channels = static_cast<Uint8>(info.channels);
		ch.spec.format = AUDIO_S16SYS;
		ch.streamType = ChannelStreamType::OggFile;
	}
	else if (ext == ".mp3") {
		drmp3* mp3 = static_cast<drmp3*>(SDL_calloc(1, sizeof(drmp3)));

		if (!mp3 || !drmp3_init_file(mp3, path.c_str(), nullptr)) {
			if (mp3) SDL_free(mp3);
			backend->GetPlatform()->Log("Failed to stream MP3: " + path);
			StopSample(channel, true);
			return;
		}

		ch.decoder = mp3;
		ch.streamSampleRate = static_cast<int>(mp3->sampleRate);
		ch.streamChannels = static_cast<int>(mp3->channels);
		ch.spec.freq = ch.streamSampleRate;
		ch.spec.channels = static_cast<Uint8>(ch.streamChannels);
		ch.spec.format = AUDIO_S16SYS;
		ch.streamType = ChannelStreamType::Mp3File;
	}
	else {
		backend->GetPlatform()->Log("Streaming only supports OGG/MP3: " + path);
		StopSample(channel, true);
		return;
	}

	ch.stream = SDL_NewAudioStream(
		ch.spec.format,
		ch.spec.channels,
		ch.spec.freq,
		spec.format,
		spec.channels,
		spec.freq
	);

	if (!ch.stream) {
		backend->GetPlatform()->Log("SDL_NewAudioStream error: " + std::string(SDL_GetError()));
		StopSample(channel, true);
		return;
	}

	ch.decodeBuffer.resize(8192 * ch.spec.channels);

	// Prebuffer a bit so playback does not start empty.
	for (int i = 0; i < 4; ++i)
		RefillStreamingChannel(ch);
}
void SDL2AudioBackend::RefillStreamingChannel(Channel& ch)
{
	if (!ch.streaming || !ch.stream || ch.pause)
		return;

	const int maxQueuedBytes = 256 * 1024;

	while (SDL_AudioStreamAvailable(ch.stream) < maxQueuedBytes && !ch.sourceEnded) {
		int framesRead = 0;

		if (ch.streamType == ChannelStreamType::OggFile) {
			auto* ogg = static_cast<stb_vorbis*>(ch.decoder);

			framesRead = stb_vorbis_get_samples_short_interleaved(
				ogg,
				ch.spec.channels,
				ch.decodeBuffer.data(),
				static_cast<int>(ch.decodeBuffer.size())
			);
		}
		else if (ch.streamType == ChannelStreamType::Mp3File) {
			auto* mp3 = static_cast<drmp3*>(ch.decoder);

			framesRead = static_cast<int>(
				drmp3_read_pcm_frames_s16(mp3, 8192, ch.decodeBuffer.data())
			);
		}

		if (framesRead <= 0) {
			if (ch.loop || ch.loopsRemaining > 1) {
				if (ch.loopsRemaining > 1)
					ch.loopsRemaining--;

				if (ch.streamType == ChannelStreamType::OggFile) {
					stb_vorbis_seek_start(static_cast<stb_vorbis*>(ch.decoder));
				}
				else if (ch.streamType == ChannelStreamType::Mp3File) {
					drmp3_seek_to_pcm_frame(static_cast<drmp3*>(ch.decoder), 0);
				}

				continue;
			}

			ch.sourceEnded = true;
			break;
		}

		const int bytes = framesRead * ch.spec.channels * sizeof(int16_t);
		SDL_AudioStreamPut(ch.stream, ch.decodeBuffer.data(), bytes);
	}
}

void SDL2AudioBackend::DiscardSampleFile(std::string path)
{
	AudioDeviceLock lock(audio_device);
	auto it = sampleFiles.find(path);
	if (it == sampleFiles.end()) {
		backend->GetPlatform()->Log("Can't find sample path.");
		return;
	}
	if (it->second.data) SDL_free(it->second.data);
	sampleFiles.erase(it);
}

bool SDL2AudioBackend::SampleState(int id, bool channel, bool pauseOrStop)
{
	AudioDeviceLock lock(audio_device);

	if (id == -1 && !channel && !pauseOrStop) {
		for (int i = 1; i < static_cast<int>(SDL_arraysize(channels)); ++i) {
			if (channels[i].stream) return false;
		}
		return true;
	}

	if (channel) {
		if (id < 1 || id >= static_cast<int>(SDL_arraysize(channels))) return false;
		return pauseOrStop ? channels[id].pause : !channels[id].stream;
	}

	if (id > -1 && !channel) {
		bool found = false;
		for (int i = 1; i < static_cast<int>(SDL_arraysize(channels)); ++i) {
			if (channels[i].curHandle == id) {
				found = true;
				if (pauseOrStop && channels[i].pause) return true;
				if (!pauseOrStop && channels[i].stream) return false;
			}
		}
		return pauseOrStop ? false : !found;
	}

	return false;
}

void SDL2AudioBackend::PauseSample(int id, bool channel, bool pause)
{
	AudioDeviceLock lock(audio_device);

	if (id == -1 && !channel) {
		for (int i = 1; i < static_cast<int>(SDL_arraysize(channels)); ++i) channels[i].pause = pause;
		return;
	}

	if (channel) {
		if (id < 1 || id >= static_cast<int>(SDL_arraysize(channels))) return;
		if (channels[id].stream) channels[id].pause = pause;
		return;
	}

	if (id > -1 && !channel) {
		for (int i = 1; i < static_cast<int>(SDL_arraysize(channels)); ++i) {
			if (channels[i].curHandle == id) channels[i].pause = pause;
		}
	}
}

void SDL2AudioBackend::SetSamplePan(float pan, int id, bool channel)
{
	AudioDeviceLock lock(audio_device);
	pan = SDL_clamp(pan / 100.0f, -1.0f, 1.0f);

	if (!channel && id <= -1) {
		mainPan = pan;
		for (int i = 1; i < static_cast<int>(SDL_arraysize(channels)); ++i) channels[i].pan = mainPan;
		return;
	}

	if (channel) {
		if (id < 1 || id >= static_cast<int>(SDL_arraysize(channels))) return;
		channels[id].pan = pan;
		return;
	}

	if (id > -1 && !channel) {
		for (int i = 1; i < static_cast<int>(SDL_arraysize(channels)); ++i) {
			if (channels[i].curHandle == id) channels[i].pan = pan;
		}
	}
}

int SDL2AudioBackend::GetSamplePan(int id, bool channel)
{
	AudioDeviceLock lock(audio_device);
	if (id == -1 && !channel) return static_cast<int>(mainPan * 100.0f);
	if (channel) {
		if (id < 1 || id >= static_cast<int>(SDL_arraysize(channels))) return 0;
		return static_cast<int>(channels[id].pan * 100.0f);
	}
	if (!channel && id >= 0) {
		for (int i = 1; i < static_cast<int>(SDL_arraysize(channels)); ++i) {
			if (channels[i].curHandle == id) return static_cast<int>(channels[i].pan * 100.0f);
		}
	}
	return 0;
}

void SDL2AudioBackend::SetSamplePos(int pos, int id, bool channel)
{
	AudioDeviceLock lock(audio_device);

	if (channel) {
		if (id < 1 || id >= static_cast<int>(SDL_arraysize(channels))) return;
		Channel& ch = channels[id];
		if (!ch.data || !ch.stream) return;

		const int bytesPerFrame = (SDL_AUDIO_BITSIZE(ch.spec.format) / 8) * ch.spec.channels;
		if (bytesPerFrame <= 0) return;

		const int lengthFrames = static_cast<int>(ch.data_len / bytesPerFrame);
		pos = SDL_clamp(pos, 0, lengthFrames);
		ch.position = pos;

		SDL_AudioStreamClear(ch.stream);
		Uint8* positionData = ch.data + pos * bytesPerFrame;
		Uint32 positionLength = ch.data_len - pos * bytesPerFrame;
		SDL_AudioStreamPut(ch.stream, positionData, positionLength);
		return;
	}

	if (id < 0) return;
	for (int i = 1; i < static_cast<int>(SDL_arraysize(channels)); ++i) {
		if (channels[i].curHandle == id) SetSamplePos(pos, i, true);
	}
}

void SDL2AudioBackend::SetSampleVolume(float volume, int id, bool channel)
{
	AudioDeviceLock lock(audio_device);

	if (id == -1 && !channel) {
		mainVol = volume;
		for (int i = 1; i < static_cast<int>(SDL_arraysize(channels)); ++i) SetSampleVolume(volume, i, true);
		return;
	}

	if (channel) {
		if (id < 1 || id >= static_cast<int>(SDL_arraysize(channels))) return;
		channels[id].volume = SDL_clamp(volume / 100.0f, 0.0f, 1.0f);
		return;
	}

	if (id > -1 && !channel) {
		for (int i = 1; i < static_cast<int>(SDL_arraysize(channels)); ++i) {
			if (channels[i].curHandle == id) SetSampleVolume(volume, i, true);
		}
	}
}

void SDL2AudioBackend::SetSampleFreq(int freq, int id, bool channel)
{
	(void)freq; (void)id; (void)channel;
	// SDL2 SDL_AudioStream has no SDL3-style frequency-ratio API.
}

int SDL2AudioBackend::GetSampleFreq(int id, bool channel)
{
	AudioDeviceLock lock(audio_device);
	if (channel) {
		if (id < 1 || id >= static_cast<int>(SDL_arraysize(channels))) return 0;
		return channels[id].spec.freq;
	}
	if (id > -1 && !channel) {
		for (int i = 1; i < static_cast<int>(SDL_arraysize(channels)); ++i) {
			if (channels[i].curHandle == id) return channels[i].spec.freq;
		}
	}
	return 0;
}

int SDL2AudioBackend::GetSampleVolume(int id)
{
	AudioDeviceLock lock(audio_device);
	if (id == -1) return static_cast<int>(mainVol);
	if (id > -1) {
		for (int i = 1; i < static_cast<int>(SDL_arraysize(channels)); ++i) {
			if (channels[i].curHandle == id && channels[i].stream) return static_cast<int>(channels[i].volume * 100.0f);
		}
	}
	return 0;
}

int SDL2AudioBackend::GetSampleVolume(std::string name)
{
	return GetSampleVolume(FindSample(name));
}

int SDL2AudioBackend::GetChannelVolume(int id)
{
	AudioDeviceLock lock(audio_device);
	if (id < 1 || id >= static_cast<int>(SDL_arraysize(channels))) return -1;
	return static_cast<int>(channels[id].volume * 100.0f);
}

std::string SDL2AudioBackend::GetChannelName(int channel)
{
	AudioDeviceLock lock(audio_device);
	if (channel < 0 || channel >= static_cast<int>(SDL_arraysize(channels))) return "";
	return channels[channel].name;
}

void SDL2AudioBackend::LockChannel(int channel, bool unlock)
{
	AudioDeviceLock lock(audio_device);
	if (channel < 0 || channel >= static_cast<int>(SDL_arraysize(channels))) return;
	channels[channel].lock = !unlock;
}

int SDL2AudioBackend::GetSampleDuration(int id, bool channel)
{
	AudioDeviceLock lock(audio_device);

	auto durationForChannel = [&](int ch) -> int {
		if (ch < 1 || ch >= static_cast<int>(SDL_arraysize(channels))) return 0;
		const Channel& c = channels[ch];
		if (!c.data || c.data_len == 0) return 0;
		const int bytesPerFrame = (SDL_AUDIO_BITSIZE(c.spec.format) / 8) * c.spec.channels;
		if (bytesPerFrame <= 0 || c.spec.freq <= 0) return 0;
		const int frames = static_cast<int>(c.data_len / bytesPerFrame);
		return static_cast<int>((static_cast<double>(frames) / static_cast<double>(c.spec.freq)) * 1000.0);
	};

	if (channel) return durationForChannel(id);
	if (!channel && id > -1) {
		for (int i = 1; i < static_cast<int>(SDL_arraysize(channels)); ++i) {
			if (channels[i].curHandle == id) return durationForChannel(i);
		}
	}
	return 0;
}

int SDL2AudioBackend::GetSamplePos(int id, bool channel)
{
	AudioDeviceLock lock(audio_device);
	if (channel) {
		if (id > 0 && id < static_cast<int>(SDL_arraysize(channels))) return channels[id].position;
		return 0;
	}
	if (!channel && id > -1) {
		for (int i = 1; i < static_cast<int>(SDL_arraysize(channels)); ++i) {
			if (channels[i].curHandle == id) return channels[i].position;
		}
	}
	return 0;
}

void SDL2AudioBackend::StopSample(int id, bool channel)
{
	AudioDeviceLock lock(audio_device);
	StopSampleUnlocked(id, channel);
}

void SDL2AudioBackend::StopSampleUnlocked(int id, bool channel)
{
	if (channel)
	{
		if (id < 1 ||
			id >= static_cast<int>(SDL_arraysize(channels)))
		{
			return;
		}

		Channel& ch = channels[id];

		if (ch.decoder)
		{
			if (ch.streamType == ChannelStreamType::OggFile)
			{
				stb_vorbis_close(
					static_cast<stb_vorbis*>(ch.decoder)
				);
			}
			else if (ch.streamType == ChannelStreamType::Mp3File)
			{
				auto* mp3 = static_cast<drmp3*>(ch.decoder);

				drmp3_uninit(mp3);
				SDL_free(mp3);
			}

			ch.decoder = nullptr;
		}

		if (ch.stream)
		{
			SDL_AudioStreamClear(ch.stream);
			SDL_FreeAudioStream(ch.stream);
			ch.stream = nullptr;
		}

		if (ch.data)
		{
			SDL_free(ch.data);
			ch.data = nullptr;
		}

		ch.compressedStreamData.clear();
		ch.data_len = 0;
		ch.position = 0;
		ch.finished = false;
		ch.pause = false;
		ch.loop = false;
		ch.streaming = false;
		ch.sourceEnded = false;
		ch.loopsRemaining = 0;
		ch.streamType = ChannelStreamType::None;
		ch.streamPath.clear();
		ch.decodeBuffer.clear();
		ch.curHandle = -1;
		ch.name.clear();

		return;
	}

	for (int i = 1;
		i < static_cast<int>(SDL_arraysize(channels));
		++i)
	{
		if (id == -1 || channels[i].curHandle == id)
		{
			StopSampleUnlocked(i, true);
		}
	}
}

void SDL2AudioBackend::UpdateSample()
{
	AudioDeviceLock lock(audio_device);

	for (int i = 1; i < static_cast<int>(SDL_arraysize(channels)); ++i) {
		Channel& ch = channels[i];
		ch.volume = SDL_clamp(ch.volume, 0.0f, 1.0f);

		if (!ch.stream)
			continue;

		if (ch.streaming) {
			RefillStreamingChannel(ch);

			if (ch.sourceEnded && SDL_AudioStreamAvailable(ch.stream) <= 0) {
				StopSampleUnlocked(i, true);
			}

			continue;
		}

		if (ch.finished) {
			ch.finished = false;

			if (!ch.loop) {
				StopSampleUnlocked(i, true);
				continue;
			}

			SDL_AudioStreamPut(ch.stream, ch.data, ch.data_len);
		}
	}
}
void SDL2AudioBackend::PreloadSample(int id)
{
	if (id < 0)
		return;

	if (sampleCache.find(id) != sampleCache.end())
		return;

	SoundInfo* soundInfo = SoundBank::Instance().GetSound(id);
	if (!soundInfo)
		return;

	std::vector<uint8_t> data =
		backend->platform->GetPakFile().GetData(
			"sounds/" + std::to_string(id) + "." + soundInfo->Type
		);

	if (data.empty())
		return;

	// Do not preload things your current backend wants to stream.
	if (ShouldStreamSample(id, soundInfo, data.size(), 1))
		return;

	Channel temp{};

	AudioDeviceLock lock(audio_device);

	if (!LoadSampleFromMemory(id, 0, soundInfo, data))
		return;

	Channel& decoded = channels[0];

	CachedSample cached;
	cached.data_len = decoded.data_len;
	cached.spec = decoded.spec;
	cached.name = decoded.name;

	cached.data = static_cast<Uint8*>(SDL_malloc(cached.data_len));
	if (!cached.data) {
		if (decoded.stream) {
			SDL_AudioStreamClear(decoded.stream);
			SDL_FreeAudioStream(decoded.stream);
			decoded.stream = nullptr;
		}
		if (decoded.data) {
			SDL_free(decoded.data);
			decoded.data = nullptr;
			decoded.data_len = 0;
		}
		return;
	}

	SDL_memcpy(cached.data, decoded.data, cached.data_len);

	if (decoded.stream) {
		SDL_AudioStreamClear(decoded.stream);
		SDL_FreeAudioStream(decoded.stream);
		decoded.stream = nullptr;
	}

	if (decoded.data) {
		SDL_free(decoded.data);
		decoded.data = nullptr;
		decoded.data_len = 0;
	}

	sampleCache.emplace(id, cached);
}

void SDL2AudioBackend::UnloadPreloadedSample(int id)
{
	AudioDeviceLock lock(audio_device);

	auto it = sampleCache.find(id);
	if (it == sampleCache.end())
		return;

	if (it->second.data) {
		SDL_free(it->second.data);
		it->second.data = nullptr;
	}

	sampleCache.erase(it);
}


#endif
