#pragma once

#ifdef NUCLEAR_BACKEND_SDL2

#include "GraphicsBackend.h"
#include "FontBank.h"
#include <SDL2/SDL.h>
#include <SDL_ttf.h>
#include <unordered_map>
#include <set>
#include <memory>
#include <vector>
#include <string>
#include <deque>
#include <queue>

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
	void DrawCounterBar(int x, int y, Counter* counter) override;

	void LoadFont(int id) override;
	void UnloadFont(int id) override;
	void DrawText(FontInfo *fontInfo, int x, int y, int width, int height, unsigned char horizontalAlignment, unsigned char verticalAlignment, int color, const std::string &text, int objectHandle = -1, int rgbCoefficient = 0xFFFFFF, int effect = 0, unsigned char effectParameter = 0, EffectInstance* effectInstance = nullptr) override;

	SDL_Renderer* GetRenderer() { return renderer; }
	SDL_Window*   GetWindow()   { return window; }

private:
	SDL_Window*   window   = nullptr;
	SDL_Renderer* renderer = nullptr;
	SDL2Backend*  backend  = nullptr;

	std::unordered_map<int, SDL_Texture*> mosaics;
	std::unordered_map<int, int> imageToMosaic;
	std::unordered_map<int, std::set<int>> mosaicToImages;

	std::unordered_map<int, TTF_Font*> fonts;
	std::unordered_map<std::string, std::shared_ptr<std::vector<uint8_t>>> fontBuffers;

	struct TextCacheKey {
		unsigned int fontHandle;
		std::string text;
		int color;
		int objectHandle;
		
		bool operator==(const TextCacheKey& other) const {
			return fontHandle == other.fontHandle && text == other.text && color == other.color && objectHandle == other.objectHandle;
		}
	};
	
	struct TextCacheKeyHash {
		std::size_t operator()(const TextCacheKey& key) const {
			std::size_t h1 = std::hash<unsigned int>{}(key.fontHandle);
			std::size_t h2 = std::hash<std::string>{}(key.text);
			std::size_t h3 = std::hash<int>{}(key.color);
			std::size_t h4 = std::hash<int>{}(key.objectHandle);
			return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
		}
	};
		
	struct CachedText {
		SDL_Texture* texture = nullptr;
		int width  = 0;
		int height = 0;
	};
	
	std::unordered_map<TextCacheKey, CachedText, TextCacheKeyHash> textCache;

	void RemoveOldTextCache();
	void ClearTextCacheForFont(int fontHandle);

	static SDL_Surface* LoadLZ4SurfaceFromMemory(const std::vector<uint8_t>& data);	

	SDL_Color RGBToSDLColor(int color);
	SDL_Color RGBAToSDLColor(int color);


	Uint32 fpsLastUpdate = 0;
	Uint32 fpsFrameCount = 0;
	float currentFps = 0.0f;
	std::string baseWindowTitle;
};

#endif
