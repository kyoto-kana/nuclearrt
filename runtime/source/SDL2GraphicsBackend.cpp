#ifdef NUCLEAR_BACKEND_SDL2

#include "SDL2GraphicsBackend.h"
#include "SDL2Backend.h"
#include "Application.h"
#include "Counter.h"
#include "FontBank.h"
#include "ImageBank.h"
#include "Shape.h"

#include <SDL2/SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <algorithm>
#include <cstring>
#include "lz4.h"

#define ENABLE_SUBPIXEL_RENDERING 1

SDL_Surface* SDL2GraphicsBackend::LoadLZ4SurfaceFromMemory(const std::vector<uint8_t>& data)
{
	if (data.size() < sizeof(uint16_t) * 2 + sizeof(uint32_t) + sizeof(int32_t))
	{
		Application::Instance().GetBackend().get()->platform->Log("LoadLZ4SurfaceFromMemory: data too small to contain header");
		return nullptr;
	}

	const uint8_t* ptr = data.data();

	uint16_t width  = EndiannessHelper::ReadLE16(ptr);
	uint16_t height = EndiannessHelper::ReadLE16(ptr);
	uint32_t surface_format = EndiannessHelper::ReadLE32(ptr);
	int32_t compressed_size = (int32_t)EndiannessHelper::ReadLE32(ptr);

	const uint8_t* compressed_buffer = ptr;
	size_t header_size = sizeof(width) + sizeof(height) + sizeof(surface_format) + sizeof(compressed_size);

	if (data.size() < header_size + (size_t)compressed_size) {
		Application::Instance().GetBackend().get()->platform->Log("LoadLZ4SurfaceFromMemory: data truncated expected " +
			std::to_string(header_size + compressed_size) +
			" bytes, got " +
			std::to_string(data.size()));
		return nullptr;
	}

	SDL_Surface* out = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_BGRA32);

	if (!out) {
		Application::Instance().GetBackend().get()->platform->Log("SDL_CreateRGBSurfaceWithFormat Error: " + std::string(SDL_GetError()));
		return nullptr;
	}

	uint32_t bpp = out->format->BytesPerPixel;
	int uncompressed_size = (int)(width * height * bpp);

	int result = LZ4_decompress_safe(
		reinterpret_cast<const char*>(compressed_buffer),
		reinterpret_cast<char*>(out->pixels),
		compressed_size,
		uncompressed_size);

	if (result < 0) {
		Application::Instance().GetBackend().get()->platform->Log("LZ4_decompress_safe Error: decompression failed code " + std::to_string(result));
		SDL_FreeSurface(out);
		return nullptr;
	}

	return out;
}

void SDL2GraphicsBackend::SetWindowAndRenderer(SDL_Window* win, SDL_Renderer* ren)
{
    window   = win;
    renderer = ren;
/*
	if (window) {
		baseWindowTitle = SDL_GetWindowTitle(window);
	}

	fpsLastUpdate = SDL_GetTicks();
	*/
}

void SDL2GraphicsBackend::Deinitialize()
{
    // cleanup text texture cache
    for (auto& pair : textCache) {
        if (pair.second.texture) {
            SDL_DestroyTexture(pair.second.texture);
        }
    }
    textCache.clear();

    // cleanup mosaics
    for (auto& pair : mosaics) {
        if (pair.second) {
            SDL_DestroyTexture(pair.second);
        }
    }
	mosaics.clear();
	imageToMosaic.clear();
	mosaicToImages.clear();

    // cleanup fonts
    for (auto& pair : fonts) {
        TTF_CloseFont(pair.second);
    }
    fonts.clear();
    fontBuffers.clear();

    if (renderer) { SDL_DestroyRenderer(renderer); renderer = nullptr; }
    if (window)   { SDL_DestroyWindow(window);     window   = nullptr; }
}

void SDL2GraphicsBackend::BeginDrawing() {
	int borderColor = Application::Instance().GetAppData()->GetBorderColor();
	SDL_Color c = RGBToSDLColor(borderColor);
	SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 255);
	SDL_RenderClear(renderer);
}

void SDL2GraphicsBackend::EndDrawing() {

	SDL_RenderPresent(renderer);
	/*
	fpsFrameCount++;

	const Uint32 now = SDL_GetTicks();
	const Uint32 elapsed = now - fpsLastUpdate;

	if (elapsed >= 1000) {
		currentFps = (fpsFrameCount * 1000.0f) / elapsed;

		std::string title =
			baseWindowTitle +
			" (FPS: " +
			std::to_string(static_cast<int>(currentFps)) + ")";

		SDL_SetWindowTitle(window, title.c_str());

		fpsFrameCount = 0;
		fpsLastUpdate = now;
	}
	*/
}

void SDL2GraphicsBackend::Clear(int color) {
	SDL_SetRenderDrawColor(renderer,
		(color >> 16) & 0xFF,
		(color >> 8)  & 0xFF,
		 color        & 0xFF,
		(color >> 24) & 0xFF);
	SDL_RenderClear(renderer);
}

void SDL2GraphicsBackend::LoadTexture(int id)
{
	Application::Instance().GetBackend().get()->platform->Log(
	"LoadTexture: entered image=" + std::to_string(id)
	);
	if (imageToMosaic.count(id)) return;

	auto imageInfo = ImageBank::Instance().GetImage(id);
	if (!imageInfo)
	{
		Application::Instance().GetBackend().get()->platform->Log(
			"ImageBank::GetImage Error: Image with id " + std::to_string(id) + " not found"
		);
		return;
	}

	int mosaicIndex = imageInfo->MosaicIndex;

	Application::Instance().GetBackend().get()->platform->Log(
	"LoadTexture: mosaicIndex=" + std::to_string(mosaicIndex)
	);
	
	if (mosaicIndex < 0)
	{
		Application::Instance().GetBackend().get()->platform->Log(
			"LoadTexture Error: Image with id " +
			std::to_string(id) +
			" has invalid mosaic index " +
			std::to_string(mosaicIndex)
		);
		return;
	}

	if (mosaics.find(mosaicIndex) == mosaics.end())
	{
		char mosaicFileName[64];
		std::snprintf(
			mosaicFileName,
			sizeof(mosaicFileName),
			"images/m%05d.sdl_surface.lz4",
			mosaicIndex
		);

		Application::Instance().GetBackend().get()->platform->Log(
			"LoadTexture: loading pak entry " + std::string(mosaicFileName)
		);

		std::vector<uint8_t> data = backend->platform->GetPakFile().GetData(mosaicFileName);

		Application::Instance().GetBackend().get()->platform->Log(
			"LoadTexture: pak entry loaded, size=" + std::to_string(data.size())
		);

		if (data.empty())
		{
			Application::Instance().GetBackend().get()->platform->Log(
				"PakFile::GetData Error: Mosaic " + std::string(mosaicFileName) + " not found"
			);
			return;
		}

		Application::Instance().GetBackend().get()->platform->Log(
			"LoadTexture: before LoadLZ4SurfaceFromMemory"
		);

		SDL_Surface* surface = LoadLZ4SurfaceFromMemory(data);
		Application::Instance().GetBackend().get()->platform->Log(
			"LoadTexture: after LoadLZ4SurfaceFromMemory"
		);

		if (!surface)
		{
			Application::Instance().GetBackend().get()->platform->Log(
				"LoadLZ4SurfaceFromMemory Error: failed to decode " + std::string(mosaicFileName)
			);
			return;
		}

		SDL_Texture* mosaicTexture = SDL_CreateTextureFromSurface(renderer, surface);
		
		SDL_FreeSurface(surface);

		if (!mosaicTexture)
		{
			Application::Instance().GetBackend().get()->platform->Log(
				"SDL_CreateTextureFromSurface Error: " + std::string(SDL_GetError())
			);
			return;
		}

		mosaics[mosaicIndex] = mosaicTexture;
	}

	imageToMosaic[id] = mosaicIndex;
	mosaicToImages[mosaicIndex].insert(id);
}

void SDL2GraphicsBackend::UnloadTexture(int id) {
	auto mosaicIt = imageToMosaic.find(id);
	if (mosaicIt == imageToMosaic.end()) {
		return;
	}
	
	int mosaicIndex = mosaicIt->second;
	mosaicToImages[mosaicIndex].erase(id);
	imageToMosaic.erase(mosaicIt);
	
	//if no more images use this mosaic, unload it
	if (mosaicToImages[mosaicIndex].empty()) {
		auto mosaicTextureIt = mosaics.find(mosaicIndex);
		if (mosaicTextureIt != mosaics.end()) {
			SDL_DestroyTexture(mosaicTextureIt->second);
			mosaics.erase(mosaicTextureIt);
		}
		mosaicToImages.erase(mosaicIndex);
	}
}

void SDL2GraphicsBackend::DrawTexture(int id, int x, int y, int offsetX, int offsetY,
	int angle, float scaleX, float scaleY, int color, int effect,
	unsigned char effectParameter, EffectInstance* effectInstance)
{
	(void)effectInstance;

	auto imageInfo = ImageBank::Instance().GetImage(id);
	if (!imageInfo) return;

	auto mosaicIt = imageToMosaic.find(id);
	if (mosaicIt == imageToMosaic.end()) {
		LoadTexture(id);
		mosaicIt = imageToMosaic.find(id);
		if (mosaicIt == imageToMosaic.end()) return;
	}

	auto texIt = mosaics.find(mosaicIt->second);
	if (texIt == mosaics.end() || !texIt->second) return;

	SDL_Texture* tex = texIt->second;

	SDL_Rect src = {
		imageInfo->MosaicX,
		imageInfo->MosaicY,
		imageInfo->Width,
		imageInfo->Height
	};

	int w = imageInfo->Width;
	int h = imageInfo->Height;

	Uint8 origR, origG, origB, origA;
	SDL_BlendMode origBlend;
	SDL_GetTextureColorMod(tex, &origR, &origG, &origB);
	SDL_GetTextureAlphaMod(tex, &origA);
	SDL_GetTextureBlendMode(tex, &origBlend);

	if (w == 0 || h == 0) {
		SDL_QueryTexture(tex, nullptr, nullptr, &w, &h);
	}

	if (scaleX == 0.0f) scaleX = 1.0f;
	if (scaleY == 0.0f) scaleY = 1.0f;

	// effect 1 ignores color, matching your SDL3 path
	if (effect == 1) {
		color = 0xFFFFFF;
	}

	SDL_SetTextureColorMod(
		tex,
		(color >> 16) & 0xFF,
		(color >> 8) & 0xFF,
		color & 0xFF
	);

	SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
	SDL_SetTextureAlphaMod(tex, 255);

	switch (effect) {
		case 4096:
		case 0:
			SDL_SetTextureAlphaMod(tex, 255 - effectParameter);
			break;

		case 1: // Semi-transparent
			SDL_SetTextureAlphaMod(tex, 255 - effectParameter);
			break;

		case 9: // Additive
			SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_ADD);
			SDL_SetTextureAlphaMod(tex, 255 - effectParameter);
			break;
	}

#if ENABLE_SUBPIXEL_RENDERING
	SDL_FRect rect = {
		x - offsetX * scaleX,
		y - offsetY * scaleY,
		w * scaleX,
		h * scaleY
	};

	SDL_FPoint center = {
		offsetX* scaleX,
		offsetY* scaleY
	};

	if (angle == 0)
	{
		SDL_RenderCopyF(
			renderer,
			tex,
			&src,
			&rect
		);
	}
	else
	{
		SDL_RenderCopyExF(
			renderer,
			tex,
			&src,
			&rect,
			360.0 - static_cast<double>(angle),
			&center,
			SDL_FLIP_NONE
		);
	}
#else
	SDL_Rect rect = {
		static_cast<int>(x - (offsetX * scaleX)),
		static_cast<int>(y - (offsetY * scaleY)),
		static_cast<int>(w * scaleX),
		static_cast<int>(h * scaleY)
	};

	SDL_Point center = {
		static_cast<int>(offsetX * scaleX),
		static_cast<int>(offsetY * scaleY)
	};

	if (angle == 0)
	{
		SDL_RenderCopy(renderer, tex, &src, &rect);
	}
	else
	{
		SDL_RenderCopyEx(renderer, tex, &src, &rect, 360 - angle, &center, SDL_FLIP_NONE);
	}
#endif

	SDL_SetTextureColorMod(tex, origR, origG, origB);
	SDL_SetTextureAlphaMod(tex, origA);
	SDL_SetTextureBlendMode(tex, origBlend);
}

void SDL2GraphicsBackend::DrawQuickBackdrop(int x, int y, int width, int height, Shape* shape) {
	if (shape->ShapeType == 1) { // Line
		SDL_SetRenderDrawColor(renderer,
			(shape->BorderColor >> 16) & 0xFF,
			(shape->BorderColor >> 8)  & 0xFF,
			 shape->BorderColor        & 0xFF, 255);
		int x1 = shape->FlipX ? x + width  : x;
		int y1 = shape->FlipY ? y + height : y;
		int x2 = shape->FlipX ? x          : x + width;
		int y2 = shape->FlipY ? y           : y + height;
		SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
	} else {
		if (shape->FillType == 1) { // Solid
			SDL_SetRenderDrawColor(renderer,
				(shape->Color1 >> 16) & 0xFF,
				(shape->Color1 >> 8)  & 0xFF,
				 shape->Color1        & 0xFF, 255);
			SDL_Rect rect = { x, y, width, height };
			SDL_RenderFillRect(renderer, &rect);
		} else if (shape->FillType == 2) { // Gradient
			Uint8 r1=(shape->Color1>>16)&0xFF, g1=(shape->Color1>>8)&0xFF, b1=shape->Color1&0xFF;
			Uint8 r2=(shape->Color2>>16)&0xFF, g2=(shape->Color2>>8)&0xFF, b2=shape->Color2&0xFF;
			if (shape->VerticalGradient) {
				for (int i = 0; i < height; i++) {
					float t = (float)i / height;
					SDL_SetRenderDrawColor(renderer,
						(Uint8)(r1+(r2-r1)*t), (Uint8)(g1+(g2-g1)*t), (Uint8)(b1+(b2-b1)*t), 255);
					SDL_RenderDrawLine(renderer, x, y+i, x+width-1, y+i);
				}
			} else {
				for (int i = 0; i < width; i++) {
					float t = (float)i / width;
					SDL_SetRenderDrawColor(renderer,
						(Uint8)(r1+(r2-r1)*t), (Uint8)(g1+(g2-g1)*t), (Uint8)(b1+(b2-b1)*t), 255);
					SDL_RenderDrawLine(renderer, x+i, y, x+i, y+height-1);
				}
			}
		} else if (shape->FillType == 3) { // Motif
			auto imageInfo = ImageBank::Instance().GetImage(shape->Image);

			if (!imageInfo)
			{
				backend->GetPlatform()->Log(
					"ImageBank miss for image " +
					std::to_string(shape->Image)
				);
				return;
			}
			if (shape->Image <= 0)
			{
				backend->GetPlatform()->Log(
					"Invalid backdrop image handle: " +
					std::to_string(shape->Image)
				);
				return;
			}
			
			auto mosaicIt = imageToMosaic.find(shape->Image);
			if (mosaicIt == imageToMosaic.end()) {
				LoadTexture(shape->Image);
				mosaicIt = imageToMosaic.find(shape->Image);
				if (mosaicIt == imageToMosaic.end()) return;
			}

			auto texIt = mosaics.find(mosaicIt->second);
			if (texIt == mosaics.end() || !texIt->second) return;

			SDL_Texture* tex = texIt->second;

			int tw = imageInfo->Width;
			int th = imageInfo->Height;
			if (tw <= 0 || th <= 0) return;

			for (int ty = y; ty < y + height; ty += th ) {
				for (int tx = x; tx < x + width; tx += tw ) {
					int dw = std::min(tw, x + width  - tx);
					int dh = std::min(th, y + height - ty);

					SDL_Rect src = {
						imageInfo->MosaicX,
						imageInfo->MosaicY,
						dw,
						dh
					};

					SDL_Rect dst = { tx, ty, dw, dh };

					SDL_RenderCopy(renderer, tex, &src, &dst);
				}
			}
		}
	}
}

void SDL2GraphicsBackend::DrawCounterBar(int x, int y, Counter* counter)
{
	if (!counter || counter->Width <= 0 || counter->Height <= 0)
		return;

	bool isVertical = counter->DisplayType == 2;

	float fillPercent = counter->MaxValue > 0
		? static_cast<float>(counter->GetValue()) / static_cast<float>(counter->MaxValue)
		: 0.0f;

	fillPercent = SDL_clamp(fillPercent, 0.0f, 1.0f);

	int fillWidth  = counter->Width;
	int fillHeight = counter->Height;
	int fillX = x;
	int fillY = y;

	if (isVertical) {
		fillHeight = static_cast<int>(counter->Height * fillPercent);
		if (counter->BarDirection == 1)
			fillY = y + (counter->Height - fillHeight);
	} else {
		fillWidth = static_cast<int>(counter->Width * fillPercent);
		if (counter->BarDirection == 1)
			fillX = x + (counter->Width - fillWidth);
	}

	if (fillWidth <= 0 || fillHeight <= 0)
		return;

	int color1 = counter->shape.Color1;
	int color2 = counter->shape.FillType == 2 ? counter->shape.Color2 : color1;

	if (counter->BarDirection) {
		int temp = color1;
		color1 = color2;
		color2 = temp;
	}

	SDL_Rect rect = { fillX, fillY, fillWidth, fillHeight };

	if (counter->shape.FillType != 2) {
		SDL_SetRenderDrawColor(
			renderer,
			(color1 >> 16) & 0xFF,
			(color1 >> 8) & 0xFF,
			color1 & 0xFF,
			255
		);
		SDL_RenderFillRect(renderer, &rect);
		return;
	}

	Uint8 r1 = (color1 >> 16) & 0xFF;
	Uint8 g1 = (color1 >> 8) & 0xFF;
	Uint8 b1 = color1 & 0xFF;

	Uint8 r2 = (color2 >> 16) & 0xFF;
	Uint8 g2 = (color2 >> 8) & 0xFF;
	Uint8 b2 = color2 & 0xFF;

	if (counter->shape.VerticalGradient) {
		for (int i = 0; i < fillHeight; i++) {
			float t = fillHeight > 1 ? static_cast<float>(i) / static_cast<float>(fillHeight - 1) : 0.0f;

			SDL_SetRenderDrawColor(
				renderer,
				static_cast<Uint8>(r1 + (r2 - r1) * t),
				static_cast<Uint8>(g1 + (g2 - g1) * t),
				static_cast<Uint8>(b1 + (b2 - b1) * t),
				255
			);

			SDL_RenderDrawLine(renderer, fillX, fillY + i, fillX + fillWidth - 1, fillY + i);
		}
	} else {
		for (int i = 0; i < fillWidth; i++) {
			float t = fillWidth > 1 ? static_cast<float>(i) / static_cast<float>(fillWidth - 1) : 0.0f;

			SDL_SetRenderDrawColor(
				renderer,
				static_cast<Uint8>(r1 + (r2 - r1) * t),
				static_cast<Uint8>(g1 + (g2 - g1) * t),
				static_cast<Uint8>(b1 + (b2 - b1) * t),
				255
			);

			SDL_RenderDrawLine(renderer, fillX + i, fillY, fillX + i, fillY + fillHeight - 1);
		}
	}
}

void SDL2GraphicsBackend::LoadFont(int id) {
	if (fonts.count(id)) return;

	FontInfo* info = FontBank::Instance().GetFont(id);
	if (!info) { std::cerr << "LoadFont: font " << id << " not found\n"; return; }

	std::shared_ptr<std::vector<uint8_t>> buf;
	if (fontBuffers.count(info->FontFileName)) {
		buf = fontBuffers[info->FontFileName];
	} else {
		buf = std::make_shared<std::vector<uint8_t>>(
			backend->GetPlatform()->GetPakFileEntryData("fonts/" + info->FontFileName));
		if (buf->empty()) { std::cerr << "LoadFont: font file not found\n"; return; }
		fontBuffers[info->FontFileName] = buf;
	}

	SDL_RWops* stream = SDL_RWFromMem(buf->data(), (int)buf->size());
	TTF_Font* font = TTF_OpenFontRW(stream, 1, info->Height);
	if (!font) { std::cerr << "TTF_OpenFontRW: " << TTF_GetError() << "\n"; return; }

	int style = TTF_STYLE_NORMAL;
	if (info->Weight > 500) style |= TTF_STYLE_BOLD;
	if (info->Italic)       style |= TTF_STYLE_ITALIC;
	if (info->Underline)    style |= TTF_STYLE_UNDERLINE;
	if (info->Strikeout)    style |= TTF_STYLE_STRIKETHROUGH;
	TTF_SetFontStyle(font, style);

	fonts[id] = font;
}

void SDL2GraphicsBackend::UnloadFont(int id) {
	auto it = fonts.find(id);
	if (it == fonts.end()) return;

	FontInfo* info = FontBank::Instance().GetFont(id);
	if (info) {
		bool shared = false;
		for (auto& [fid, _] : fonts) {
			if (fid == id) continue;
			FontInfo* other = FontBank::Instance().GetFont(fid);
			if (other && other->FontFileName == info->FontFileName) { shared = true; break; }
		}
		if (!shared) fontBuffers.erase(info->FontFileName);
	}

	ClearTextCacheForFont(id);

	TTF_CloseFont(it->second);
	fonts.erase(it);
}

void SDL2GraphicsBackend::DrawText(FontInfo* fontInfo, int x, int y, int width, int height,
    unsigned char horizontalAlignment, unsigned char verticalAlignment,
    int color, const std::string& text, int objectHandle, int rgbCoefficient,
    int effect, unsigned char effectParameter, EffectInstance* effectInstance)
{
    if (fontInfo == nullptr) return;
    if (fonts.find(fontInfo->Handle) == fonts.end()) return;

    TTF_Font* font = fonts[fontInfo->Handle];
    if (!font) return;

    // Check cache
    TextCacheKey cacheKey{ fontInfo->Handle, text, color, objectHandle };
    auto cacheIt = textCache.find(cacheKey);

    SDL_Texture* tex = nullptr;
    int textureWidth = 0;
    int textureHeight = 0;

    if (cacheIt != textCache.end()) {
        tex = cacheIt->second.texture;
        textureWidth = cacheIt->second.width;
        textureHeight = cacheIt->second.height;
    } else {
        if (objectHandle != -1) {
            auto it = textCache.begin();
            while (it != textCache.end()) {
                if (it->first.objectHandle == objectHandle) {
                    if (it->second.texture) SDL_DestroyTexture(it->second.texture);
                    it = textCache.erase(it);
                } else {
                    ++it;
                }
            }
        }

        std::string t = text;
        t.erase(std::remove(t.begin(), t.end(), '\r'), t.end());
        for (size_t i = 0; i < t.size(); i++)
            if (t[i] == '\t') { t.replace(i, 1, "    "); i += 3; }
        if (t.find_first_not_of(" \n\r\t") == std::string::npos) return;

        SDL_Surface* surf = TTF_RenderUTF8_Blended_Wrapped(
            font, t.c_str(), RGBToSDLColor(color), width);
        if (!surf) {
            backend->GetPlatform()->Log("TTF_RenderUTF8_Blended_Wrapped Error: " + std::string(TTF_GetError()));
            return;
        }

        tex = SDL_CreateTextureFromSurface(renderer, surf);
        textureWidth  = surf->w;
        textureHeight = surf->h;
        SDL_FreeSurface(surf);

        if (!tex) {
            backend->GetPlatform()->Log("SDL_CreateTextureFromSurface Error: " + std::string(SDL_GetError()));
            return;
        }

        if (textCache.size() >= 256) RemoveOldTextCache();

        CachedText cached;
        cached.texture = tex;
        cached.width   = textureWidth;
        cached.height  = textureHeight;
        textCache[cacheKey] = cached;
    }

    int drawX = x;
    if (horizontalAlignment == 1) {        // Center
        drawX += (width - textureWidth) / 2;
    } else if (horizontalAlignment == 2) { // Right
        drawX += width - textureWidth;
    }

    int drawY = y;
    if (verticalAlignment == 1) {          // Center
        drawY += (height - textureHeight) / 2;
    } else if (verticalAlignment == 2) {   // Bottom
        drawY += height - textureHeight;
    }

    SDL_Rect rect = { drawX, drawY, textureWidth, textureHeight };
    SDL_RenderCopy(renderer, tex, nullptr, &rect);
}

void SDL2GraphicsBackend::RemoveOldTextCache()
{
    if (textCache.empty()) return;

    auto oldestIt = textCache.begin();
    if (oldestIt->second.texture) {
        SDL_DestroyTexture(oldestIt->second.texture);
    }
    textCache.erase(oldestIt);
}

void SDL2GraphicsBackend::ClearTextCacheForFont(int fontHandle)
{
    auto it = textCache.begin();
    while (it != textCache.end()) {
        if (it->first.fontHandle == fontHandle) {
            if (it->second.texture) {
                SDL_DestroyTexture(it->second.texture);
            }
            it = textCache.erase(it);
        } else {
            ++it;
        }
    }
}

SDL_Color SDL2GraphicsBackend::RGBToSDLColor(int color) {
	return { (Uint8)((color>>16)&0xFF), (Uint8)((color>>8)&0xFF), (Uint8)(color&0xFF), 255 };
}

SDL_Color SDL2GraphicsBackend::RGBAToSDLColor(int color) {
	return { (Uint8)((color>>16)&0xFF), (Uint8)((color>>8)&0xFF),
	         (Uint8)(color&0xFF), (Uint8)((color>>24)&0xFF) };
}

#endif
