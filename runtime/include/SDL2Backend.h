#pragma once

#ifdef NUCLEAR_BACKEND_SDL2

#include "Backend.h"
#include "SDL2PlatformBackend.h"
#include "SDL2GraphicsBackend.h"
#include "SDL2AudioBackend.h"
#include "SDL2InputBackend.h"

#include <set>
#include <SDL2/SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <chrono>
#include <thread>

#ifdef PLATFORM_3DS
    #include <3ds.h>
#endif

#ifdef PLATFORM_SWITCH
    #include <switch.h>
#endif

#ifdef PLATFORM_WII
	#include <fat.h>
#endif

#include "Application.h"

class SDL2Backend : public Backend {
public:
	SDL2Backend() {
		int logicalWidth  = Application::Instance().GetAppData()->GetWindowWidth();
		int logicalHeight = Application::Instance().GetAppData()->GetWindowHeight();

		int outputWidth  = logicalWidth;
		int outputHeight = logicalHeight;

		#ifdef PLATFORM_VITA
		outputWidth  = 960;
		outputHeight = 544;
		#elif defined(PLATFORM_PSP)
		outputWidth  = 480;
		outputHeight = 272;
		#elif defined(PLATFORM_PS2)
		outputWidth  = 640;
		outputHeight = 480;
		#elif defined(PLATFORM_3DS)
		outputWidth  = 400;
		outputHeight = 240;
		#elif defined(PLATFORM_WIIU)
		outputWidth  = 1280;
		outputHeight = 720;
		#elif defined(PLATFORM_WII)
		outputWidth  = 640;
		outputHeight = 528;
		#endif

		std::string windowTitle = Application::Instance().GetAppData()->GetAppName();

#if defined(PLATFORM_SWITCH) || defined(PLATFORM_3DS)
   		romfsInit();
#endif



#ifdef PLATFORM_WII
		if (!fatInitDefault()) {
			printf("Unable to initialize libfat!\n");
			return;
		}
#endif


#if defined(PLATFORM_PS2) || defined(PLATFORM_VITA) // audio doesnt work for some reason
		if (SDL_Init(SDL_INIT_VIDEO) < 0) {
#else
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) < 0) {
#endif
			SDL_Log("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
			return;
		}

		if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG) {
			SDL_Log("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
			return;
		}
		if (TTF_Init() == -1) {
			SDL_Log("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
			return;
		}

		SDL_Window* win = SDL_CreateWindow(
			windowTitle.c_str(),
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    		outputWidth, outputHeight,
			SDL_WINDOW_SHOWN
		);
		SDL_Renderer* ren = SDL_CreateRenderer(
			win,
			-1,
		#ifdef PLATFORM_3DS
			SDL_RENDERER_SOFTWARE
		#else
			SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
		#endif
		);
		#ifdef PLATFORM_WII
			SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

			SDL_RenderSetLogicalSize(ren, logicalWidth, logicalHeight);

		#else
			SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
			SDL_RenderSetLogicalSize(ren, logicalWidth, logicalHeight);
		#endif

		#ifdef __wiiu__
			SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "1");
			SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "1");
		#endif


		auto* plt = new SDL2PlatformBackend();
		plt->Log("Test");
		auto* gfx = new SDL2GraphicsBackend();
		auto* aud = new SDL2AudioBackend();
		auto* inp = new SDL2InputBackend();

		plt->SetBackend(this);
		gfx->SetBackend(this);
		aud->SetBackend(this);
		inp->SetBackend(this);

		#ifdef __wiiu__
			SDL_GameController* wiiuGamepad = SDL_GameControllerOpen(0);
			if (wiiuGamepad) inp->gamepads.push_back(wiiuGamepad);
		#endif

		// Graphics needs the window/renderer created above
		gfx->SetWindowAndRenderer(win, ren);

		// Load pak file via platform backend
		#if defined(PLATFORM_WIIU)
			// some platforms dont like loading from a directory?
			if (!plt->GetPakFile().LoadFile(plt->GetAssetsDirectory() + "assets.pak")) {
				plt->Log("PakFile::LoadFile Error: Failed to load /vol/content/assets.pak");
			}
		#else
			if (!plt->GetPakFile().Load(plt->GetAssetsDirectory())) {
				plt->Log("PakFile::Load Error: Failed to load pak directory");
			}
		#endif

		platform = plt;
		graphics = gfx;
		audio    = aud;
		input    = inp;

		platform->Initialize();
		graphics->Initialize();
		audio->Initialize();
		input->Initialize();
	}

	~SDL2Backend() {
		delete input;
		delete audio;
		delete graphics;
		delete platform;

		TTF_Quit();
		IMG_Quit();
		SDL_Quit();
	}

	SDL2GraphicsBackend* GetGraphics() const { return static_cast<SDL2GraphicsBackend*>(graphics); }
	SDL2AudioBackend*    GetAudio()    const { return static_cast<SDL2AudioBackend*>(audio); }
	SDL2InputBackend*    GetInput()    const { return static_cast<SDL2InputBackend*>(input); }
	SDL2PlatformBackend* GetPlatform() const { return static_cast<SDL2PlatformBackend*>(platform); }
};

#endif
