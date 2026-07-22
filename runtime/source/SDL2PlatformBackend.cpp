#ifdef NUCLEAR_BACKEND_SDL2

#include "SDL2PlatformBackend.h"
#include "SDL2Backend.h"
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#ifdef __SWITCH__
#include <switch.h>
#endif

#ifdef PLATFORM_WIIU
#include <cstdio>
#include <vpad/input.h>
#endif


#ifdef PLATFORM_WII
#include <fat.h>
#endif

static bool g_WiiFatReady = false;

void SDL2PlatformBackend::Initialize()
{
#ifdef PLATFORM_WII
    g_WiiFatReady = fatInitDefault();

    if (!g_WiiFatReady)
    {
        printf("Init FAT: Error!\n");
    }
    else
    {
        FILE* f = fopen("sd:/nuclearrt.log", "w");
        if (f)
        {
            fprintf(f, "NuclearRT Wii log started\n");
            fclose(f);
        }
        else
        {
            printf("Could not open fat:/nuclearrt.log\n");
        }
    }
#endif
}

void SDL2PlatformBackend::Deinitialize()
{
#ifdef PLATFORM_WII
    if (g_WiiFatReady)
    {
        fatUnmount("fat");
        g_WiiFatReady = false;
    }
#endif
}


void SDL2PlatformBackend::Log(std::string text)
{
    auto now = std::chrono::system_clock::now();

    std::time_t nowTime = std::chrono::system_clock::to_time_t(now);

    std::tm tm{};
    #ifdef _WIN32
    localtime_s(&tm, &nowTime);
    #else
    localtime_r(&nowTime, &tm);
    #endif

    std::ostringstream timestamp;
    timestamp << std::put_time(&tm, "%H:%M:%S");

    static auto lastTime = std::chrono::steady_clock::now();

    auto nowSteady = std::chrono::steady_clock::now();

    auto deltaMs =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            nowSteady - lastTime).count();

    lastTime = nowSteady;

    std::string logLine =
        "[" + timestamp.str() +
        " | +" + std::to_string(deltaMs) + "ms] " +
        text;

#ifdef PLATFORM_WIIU
    FILE* f = fopen("/vol/external01/nuclearrt.log", "a");
    if (f)
    {
        fprintf(f, "%s\n", logLine.c_str());
        fflush(f);
        fclose(f);
    }

#elif defined(PLATFORM_WII)
    if (g_WiiFatReady)
    {
        FILE* f = fopen("fat:/nuclearrt.log", "a");
        if (f)
        {
            fprintf(f, "%s\n", logLine.c_str());
            fflush(f);
            fclose(f);
            return;
        }
    }

    printf("%s\n", logLine.c_str());

#elif defined(PLATFORM_PS2)
    FILE* f = fopen("host:nuclearrt.log", "a");
    if (f)
    {
        fprintf(f, "%s\n", logLine.c_str());
        fflush(f);
        fclose(f);
    }
    printf("%s\n", logLine.c_str());
#elif defined(PLATFORM_PSP)
    FILE* f = fopen("./nuclearrt.log", "a");
    if (f)
    {
        fprintf(f, "%s\n", logLine.c_str());
        fflush(f);
        fclose(f);
    }
#elif defined(PLATFORM_3DS)
    FILE* f = fopen("sdmc:/nuclearrt.log", "a");
    if (f)
    {
        fprintf(f, "%s\n", logLine.c_str());
        fflush(f);
        fclose(f);
    }
#else
    std::cout << logLine << std::endl;
#endif
}

bool SDL2PlatformBackend::ShouldQuit() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
#if defined(__SWITCH__) || defined(__vita__) || defined(__wiiu__)
		if (event.type == SDL_FINGERDOWN || event.type == SDL_FINGERMOTION) {
			SDL_Window* win = backend->GetGraphics()->GetWindow();
			int windowWidth, windowHeight;
			SDL_GetWindowSize(win, &windowWidth, &windowHeight);
			touchX = static_cast<int>(event.tfinger.x * windowWidth);
			touchY = static_cast<int>(event.tfinger.y * windowHeight);
			touchDown = true;
		}
		else if (event.type == SDL_FINGERUP) {
			touchDown = false;
        }
#endif
		if (event.type == SDL_QUIT) {
			return true;
		}
		if (event.type == SDL_CONTROLLERDEVICEADDED) {
			SDL_GameController* controller = SDL_GameControllerOpen(event.cdevice.which);
			if (controller) backend->GetInput()->gamepads.push_back(controller);
		}
		if (event.type == SDL_CONTROLLERDEVICEREMOVED) {
			for (size_t i = 0; i < backend->GetInput()->gamepads.size(); i++) {
				SDL_GameController* controller = backend->GetInput()->gamepads.at(i);
				if (SDL_GameControllerFromInstanceID(event.cdevice.which) == controller) {
					SDL_GameControllerClose(controller);
					backend->GetInput()->gamepads.erase(backend->GetInput()->gamepads.begin() + i);
					break;
				}
			}
		}
	}
	return false;
}

std::string SDL2PlatformBackend::GetPlatformName() {
#if defined(PLATFORM_WINDOWS)
	return "Windows";
#elif defined(PLATFORM_MACOS)
	return "macOS";
#elif defined(PLATFORM_LINUX)
	return "Linux";
#elif defined(PLATFORM_SWITCH)
	return "Switch";
#elif defined(PLATFORM_WIIU)
	return "WiiU";
#elif defined(PLATFORM_WII)
	return "Wii";
#elif defined(PLATFORM_VITA)
	return "PSVita";
#elif defined(PLATFORM_PSP)
	return "PSP";
#elif defined(PLATFORM_PSP)
	return "PS2";
#elif defined(PLATFORM_3DS)
	return "3DS";
#else
	return "Unknown";
#endif
}

std::string SDL2PlatformBackend::GetAssetsDirectory() {
#if defined(PLATFORM_SWITCH)
	return "romfs:/";
#elif defined(PLATFORM_VITA)
	return "app0:";
#elif defined(PLATFORM_WIIU)
	return "/vol/content/";
#elif defined(PLATFORM_WII)
	return "sd:/nuclearrt/";
#elif defined(PLATFORM_PSP)
	return "ms0:/PSP/GAME/NuclearRT/"; 
#elif defined(PLATFORM_PS2)
	return "host:"; 
#else
 	return std::string(SDL_GetBasePath());
#endif
}

float SDL2PlatformBackend::GetTimeDelta() {
	static Uint32 previousTicks = SDL_GetTicks();
	Uint32 currentTicks = SDL_GetTicks();
	float delta = (currentTicks - previousTicks) / 1000.0f;
	previousTicks = currentTicks;
	return delta;
}

void SDL2PlatformBackend::Delay(unsigned int ms) {
	SDL_Delay(ms);
}

#endif
