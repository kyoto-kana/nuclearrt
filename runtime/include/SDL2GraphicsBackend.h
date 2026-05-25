#pragma once

#ifdef NUCLEAR_BACKEND_SDL2

#include "GraphicsBackend.h"
#include "FontBank.h"
#include <SDL2/SDL.h>
#include <SDL_ttf.h>
#include <unordered_map>
#include <memory>
#include <vector>
#include <string>

class SDL2Backend;
class EffectInstance;
class Shape;

class SDL2GraphicsBackend : public GraphicsBackend {
public:
	void SetBackend(SDL2Backend* b) { backend = b; }

	void SetWindowAndRenderer(SDL_Window* win, SDL_Renderer* ren);
	void Deinitialize() override;

	void BeginDrawing() override;
	void EndDrawing() override;
	void Clear(int color) override;

	void LoadTexture(int id) override;
	void UnloadTexture(int id) override;
	void DrawTexture(int id, int x, int y, int offsetX, int offsetY, int angle,
		float scaleX, float scaleY, int color, int effect,
		unsigned char effectParameter, EffectInstance* effectInstance = nullptr) override;
	void DrawQuickBackdrop(int x, int y, int width, int height, Shape* shape) override;

	void LoadFont(int id) override;
	void UnloadFont(int id) override;
	void DrawText(FontInfo* fontInfo, int x, int y, int color, const std::string& text,
		int objectHandle = -1, int rgbCoefficient = 0xFFFFFF, int effect = 0,
		unsigned char effectParameter = 0, EffectInstance* effectInstance = nullptr) override;

	SDL_Renderer* GetRenderer() { return renderer; }
	SDL_Window*   GetWindow()   { return window; }

private:
	SDL_Window*   window   = nullptr;
	SDL_Renderer* renderer = nullptr;
	SDL2Backend*  backend  = nullptr;

	std::unordered_map<int, SDL_Texture*> textures;
	std::unordered_map<int, TTF_Font*> fonts;
	std::unordered_map<std::string, std::shared_ptr<std::vector<uint8_t>>> fontBuffers;

	SDL_Color RGBToSDLColor(int color);
	SDL_Color RGBAToSDLColor(int color);
};

#endif
