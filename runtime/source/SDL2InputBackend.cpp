#ifdef NUCLEAR_BACKEND_SDL2

#include "SDL2InputBackend.h"
#include "SDL2Backend.h"

void SDL2InputBackend::GetKeyboardState(uint8_t* outBuffer) {
	const uint8_t* state = SDL_GetKeyboardState(nullptr);
	for (int i = 0; i < 256; i++)
		outBuffer[i] = state[FusionToSDLKey(i)];
}

void SDL2InputBackend::Update()
{
	SDL_PumpEvents();
    SDL_GameControllerUpdate();
}

int SDL2InputBackend::GetMouseX() {

#if defined(__SWITCH__) || defined(__vita__) || defined(__wiiu__)
	int appWidth = Application::Instance().GetAppData()->GetWindowWidth();

#if defined(__wiiu__)
	return (backend->GetPlatform()->touchX * appWidth) / 1280;
#else
	return backend->GetPlatform()->touchX;
#endif
#endif

	int mouseX;
	SDL_GetMouseState(&mouseX, nullptr);

	int windowWidth = Application::Instance().GetAppData()->GetWindowWidth();
	
	int actualWidth;
	SDL_GetWindowSize(backend->GetGraphics()->GetWindow(), &actualWidth, nullptr);

	if (actualWidth > 0) {
		float scaleX = (float)windowWidth / (float)actualWidth;
		return (int)(mouseX * scaleX);
	}
	return mouseX;
}

int SDL2InputBackend::GetMouseY() {

#if defined(__SWITCH__) || defined(__vita__) || defined(__wiiu__)
	int appHeight = Application::Instance().GetAppData()->GetWindowHeight();

#if defined(__wiiu__)
	return (backend->GetPlatform()->touchY * appHeight) / 720;
#else
	return backend->GetPlatform()->touchY;
#endif
#endif

	int mouseY;
	SDL_GetMouseState(nullptr, &mouseY);

	int windowHeight = Application::Instance().GetAppData()->GetWindowHeight();
	
	

	int actualHeight;
	SDL_GetWindowSize(backend->GetGraphics()->GetWindow(), nullptr, &actualHeight);

	if (actualHeight > 0) {
		float scaleY = (float)windowHeight / (float)actualHeight;
		return (int)(mouseY * scaleY);
	}
	return mouseY;
}

int SDL2InputBackend::GetMouseWheelMove() {
	SDL_Event event;
	while (SDL_PollEvent(&event))
		if (event.type == SDL_MOUSEWHEEL)
			return event.wheel.y;
	return 0;
}

uint32_t SDL2InputBackend::GetMouseState() {
#if defined(__SWITCH__) || defined(__vita__) || defined(__wiiu__)
	if (backend->GetPlatform()->touchDown)
		return SDL_BUTTON(SDL_BUTTON_LEFT);
#endif
	return SDL_GetMouseState(nullptr, nullptr);
}

void SDL2InputBackend::HideMouseCursor() { SDL_ShowCursor(SDL_DISABLE); }
void SDL2InputBackend::ShowMouseCursor() { SDL_ShowCursor(SDL_ENABLE); }

bool SDL2InputBackend::IsGamepadConnected(int index) { 
    return index >= 0 && index < gamepads.size() && gamepads.at(index) != nullptr; 
} 

uint8_t SDL2InputBackend::GetGamepadButtonState(int index)
{
	uint8_t state = 0;

	// Real SDL2 controller input
	if (IsGamepadConnected(index)) {
		Sint16 leftX = SDL_GameControllerGetAxis(gamepads.at(index), SDL_CONTROLLER_AXIS_LEFTX);
		Sint16 leftY = SDL_GameControllerGetAxis(gamepads.at(index), SDL_CONTROLLER_AXIS_LEFTY);

		if (leftY < -16384 || SDL_GameControllerGetButton(gamepads.at(index), SDL_CONTROLLER_BUTTON_DPAD_UP)) state |= 1 << 0; // Up
		if (leftY >  16384 || SDL_GameControllerGetButton(gamepads.at(index), SDL_CONTROLLER_BUTTON_DPAD_DOWN)) state |= 1 << 1; // Down
		if (leftX < -16384 || SDL_GameControllerGetButton(gamepads.at(index), SDL_CONTROLLER_BUTTON_DPAD_LEFT)) state |= 1 << 2; // Left
		if (leftX >  16384 || SDL_GameControllerGetButton(gamepads.at(index), SDL_CONTROLLER_BUTTON_DPAD_RIGHT)) state |= 1 << 3; // Right

		if (SDL_GameControllerGetButton(gamepads.at(index), SDL_CONTROLLER_BUTTON_A)) state |= 1 << 4; // A
		if (SDL_GameControllerGetButton(gamepads.at(index), SDL_CONTROLLER_BUTTON_B)) state |= 1 << 5; // B
		if (SDL_GameControllerGetButton(gamepads.at(index), SDL_CONTROLLER_BUTTON_X)) state |= 1 << 6; // X
		if (SDL_GameControllerGetButton(gamepads.at(index), SDL_CONTROLLER_BUTTON_Y)) state |= 1 << 7; // Y
	}

	return state;
}


int SDL2InputBackend::FusionToSDLKey(short key) {
	switch (key) {
		default:      return SDL_SCANCODE_UNKNOWN;
		case 0x08:    return SDL_SCANCODE_BACKSPACE;
		case 0x09:    return SDL_SCANCODE_TAB;
		case 0x0D:    return SDL_SCANCODE_RETURN;
		case 0x10:    return SDL_SCANCODE_LSHIFT;
		case 0x11:    return SDL_SCANCODE_LCTRL;
		case 0x13:    return SDL_SCANCODE_PAUSE;
		case 0x14:    return SDL_SCANCODE_CAPSLOCK;
		case 0x1B:    return SDL_SCANCODE_ESCAPE;
		case 0x20:    return SDL_SCANCODE_SPACE;
		case 0x21:    return SDL_SCANCODE_PAGEUP;
		case 0x22:    return SDL_SCANCODE_PAGEDOWN;
		case 0x23:    return SDL_SCANCODE_END;
		case 0x24:    return SDL_SCANCODE_HOME;
		case 0x25:    return SDL_SCANCODE_LEFT;
		case 0x26:    return SDL_SCANCODE_UP;
		case 0x27:    return SDL_SCANCODE_RIGHT;
		case 0x28:    return SDL_SCANCODE_DOWN;
		case 0x2D:    return SDL_SCANCODE_INSERT;
		case 0x2E:    return SDL_SCANCODE_DELETE;
		case 0x30:    return SDL_SCANCODE_0;
		case 0x31:    return SDL_SCANCODE_1;
		case 0x32:    return SDL_SCANCODE_2;
		case 0x33:    return SDL_SCANCODE_3;
		case 0x34:    return SDL_SCANCODE_4;
		case 0x35:    return SDL_SCANCODE_5;
		case 0x36:    return SDL_SCANCODE_6;
		case 0x37:    return SDL_SCANCODE_7;
		case 0x38:    return SDL_SCANCODE_8;
		case 0x39:    return SDL_SCANCODE_9;
		case 0x41:    return SDL_SCANCODE_A;
		case 0x42:    return SDL_SCANCODE_B;
		case 0x43:    return SDL_SCANCODE_C;
		case 0x44:    return SDL_SCANCODE_D;
		case 0x45:    return SDL_SCANCODE_E;
		case 0x46:    return SDL_SCANCODE_F;
		case 0x47:    return SDL_SCANCODE_G;
		case 0x48:    return SDL_SCANCODE_H;
		case 0x49:    return SDL_SCANCODE_I;
		case 0x4A:    return SDL_SCANCODE_J;
		case 0x4B:    return SDL_SCANCODE_K;
		case 0x4C:    return SDL_SCANCODE_L;
		case 0x4D:    return SDL_SCANCODE_M;
		case 0x4E:    return SDL_SCANCODE_N;
		case 0x4F:    return SDL_SCANCODE_O;
		case 0x50:    return SDL_SCANCODE_P;
		case 0x51:    return SDL_SCANCODE_Q;
		case 0x52:    return SDL_SCANCODE_R;
		case 0x53:    return SDL_SCANCODE_S;
		case 0x54:    return SDL_SCANCODE_T;
		case 0x55:    return SDL_SCANCODE_U;
		case 0x56:    return SDL_SCANCODE_V;
		case 0x57:    return SDL_SCANCODE_W;
		case 0x58:    return SDL_SCANCODE_X;
		case 0x59:    return SDL_SCANCODE_Y;
		case 0x5A:    return SDL_SCANCODE_Z;
		case 0x60:    return SDL_SCANCODE_KP_0;
		case 0x61:    return SDL_SCANCODE_KP_1;
		case 0x62:    return SDL_SCANCODE_KP_2;
		case 0x63:    return SDL_SCANCODE_KP_3;
		case 0x64:    return SDL_SCANCODE_KP_4;
		case 0x65:    return SDL_SCANCODE_KP_5;
		case 0x66:    return SDL_SCANCODE_KP_6;
		case 0x67:    return SDL_SCANCODE_KP_7;
		case 0x68:    return SDL_SCANCODE_KP_8;
		case 0x69:    return SDL_SCANCODE_KP_9;
		case 0x6A:    return SDL_SCANCODE_KP_MULTIPLY;
		case 0x6B:    return SDL_SCANCODE_KP_PLUS;
		case 0x6D:    return SDL_SCANCODE_KP_MINUS;
		case 0x6E:    return SDL_SCANCODE_KP_PERIOD;
		case 0x6F:    return SDL_SCANCODE_KP_DIVIDE;
		case 0x70:    return SDL_SCANCODE_F1;
		case 0x71:    return SDL_SCANCODE_F2;
		case 0x72:    return SDL_SCANCODE_F3;
		case 0x73:    return SDL_SCANCODE_F4;
		case 0x74:    return SDL_SCANCODE_F5;
		case 0x75:    return SDL_SCANCODE_F6;
		case 0x76:    return SDL_SCANCODE_F7;
		case 0x77:    return SDL_SCANCODE_F8;
		case 0x78:    return SDL_SCANCODE_F9;
		case 0x79:    return SDL_SCANCODE_F10;
		case 0x7A:    return SDL_SCANCODE_F11;
		case 0x7B:    return SDL_SCANCODE_F12;
		case 0x90:    return SDL_SCANCODE_NUMLOCKCLEAR;
		case 0xBA:    return SDL_SCANCODE_SEMICOLON;
		case 0xBB:    return SDL_SCANCODE_EQUALS;
		case 0xBC:    return SDL_SCANCODE_COMMA;
		case 0xBD:    return SDL_SCANCODE_MINUS;
		case 0xBE:    return SDL_SCANCODE_PERIOD;
		case 0xBF:    return SDL_SCANCODE_SLASH;
		case 0xC0:    return SDL_SCANCODE_GRAVE;
		case 0xDB:    return SDL_SCANCODE_LEFTBRACKET;
		case 0xDC:    return SDL_SCANCODE_BACKSLASH;
		case 0xDD:    return SDL_SCANCODE_RIGHTBRACKET;
		case 0xDE:    return SDL_SCANCODE_APOSTROPHE;
	}
}

#endif
