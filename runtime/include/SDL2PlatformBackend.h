#pragma once

#ifdef NUCLEAR_BACKEND_SDL2

#include "PlatformBackend.h"
#include <SDL2/SDL.h>

class SDL2Backend;

class SDL2PlatformBackend : public PlatformBackend {
public:
	void SetBackend(SDL2Backend* b) { backend = b; }

	std::string GetName() const override { return "SDL2"; }

	void Initialize() override;
	void Deinitialize() override;
	
	bool ShouldQuit() override;

	std::string GetPlatformName() override;
	std::string GetAssetsDirectory() override;

	unsigned int GetTicks() override { return SDL_GetTicks(); }
	float GetTimeDelta() override;
	void Delay(unsigned int ms) override;

	void Log(std::string text) override;

#if defined(__SWITCH__) || defined(__vita__) || defined(__wiiu__)
	int touchX = 0;
	int touchY = 0;
	bool touchDown = false;
#endif

private:
	SDL2Backend* backend = nullptr;
};

#endif
