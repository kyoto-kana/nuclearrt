#pragma once

#ifdef NUCLEAR_BACKEND_SDL2

#include "InputBackend.h"
#include <SDL2/SDL.h>

class SDL2Backend;

class SDL2InputBackend : public InputBackend {
public:
	void SetBackend(SDL2Backend* b) { backend = b; }

	void GetKeyboardState(uint8_t* outBuffer) override;
	void Initialize() override {}
	void Deinitialize() override {}
	int GetMouseX() override;
	int GetMouseY() override;
	int GetMouseWheelMove() override;
	uint32_t GetMouseState() override;
	void HideMouseCursor() override;
	void ShowMouseCursor() override;

private:
	SDL2Backend* backend = nullptr;
	int FusionToSDLKey(short key);
};

#endif
