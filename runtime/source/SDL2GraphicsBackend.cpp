#ifdef NUCLEAR_BACKEND_SDL2

#include "SDL2GraphicsBackend.h"
#include "SDL2Backend.h"
#include "Application.h"
#include "FontBank.h"
#include "ImageBank.h"
#include "Shape.h"

#include <SDL2/SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <algorithm>

void SDL2GraphicsBackend::SetWindowAndRenderer(SDL_Window* win, SDL_Renderer* ren)
{
    window   = win;
    renderer = ren;
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

    // cleanup textures
    for (auto& pair : textures) {
        if (pair.second) {
            SDL_DestroyTexture(pair.second);
        }
    }
    textures.clear();

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
}

void SDL2GraphicsBackend::Clear(int color) {
	SDL_SetRenderDrawColor(renderer,
		(color >> 16) & 0xFF,
		(color >> 8)  & 0xFF,
		 color        & 0xFF,
		(color >> 24) & 0xFF);
	SDL_RenderClear(renderer);
}

void SDL2GraphicsBackend::LoadTexture(int id) {
	if (textures.count(id)) return;

	std::vector<uint8_t> data = backend->GetPlatform()->GetPakFileEntryData(
		"images/" + std::to_string(id) + ".png");
	if (data.empty()) {
		std::cerr << "LoadTexture: image " << id << " not found\n";
		return;
	}

	SDL_RWops*  stream  = SDL_RWFromMem(data.data(), (int)data.size());
	SDL_Surface* surface = IMG_Load_RW(stream, 1);
	if (!surface) { std::cerr << "IMG_Load_RW: " << IMG_GetError() << "\n"; return; }

	SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);
	textures[id] = tex;
}

void SDL2GraphicsBackend::UnloadTexture(int id) {
	auto it = textures.find(id);
	if (it != textures.end()) {
		SDL_DestroyTexture(it->second);
		textures.erase(it);
	}
}

void SDL2GraphicsBackend::DrawTexture(int id, int x, int y, int offsetX, int offsetY,
	int angle, float scaleX, float scaleY, int color, int effect,
	unsigned char effectParameter, EffectInstance* effectInstance)
{
	(void)effectInstance;

	SDL_Texture* tex = textures[id];
	if (!tex) return;

	Uint8 origR, origG, origB, origA;
	SDL_BlendMode origBlend;
	SDL_GetTextureColorMod(tex, &origR, &origG, &origB);
	SDL_GetTextureAlphaMod(tex, &origA);
	SDL_GetTextureBlendMode(tex, &origBlend);

	std::shared_ptr<ImageInfo> info = ImageBank::Instance().GetImage(id);
	int w = info ? info->Width : 0;
	int h = info ? info->Height : 0;

	if (w == 0 || h == 0) {
		SDL_QueryTexture(tex, nullptr, nullptr, &w, &h);
	}

	if (scaleX == 0.0f) scaleX = 1.0f;
	if (scaleY == 0.0f) scaleY = 1.0f;

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

		case 1: // Semi-Transparent
			SDL_SetTextureAlphaMod(tex, 255 - effectParameter);
			break;

		case 9: // Additive
			SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_ADD);
			SDL_SetTextureAlphaMod(tex, 255 - effectParameter);
			break;
	}

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

	SDL_RenderCopyEx(
		renderer,
		tex,
		nullptr,
		&rect,
		360 - angle,
		&center,
		SDL_FLIP_NONE
	);

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
			SDL_Texture* tex = textures[shape->Image];
			if (!tex) return;
			int tw, th;
			SDL_QueryTexture(tex, nullptr, nullptr, &tw, &th);
			for (int ty = y; ty < y + height; ty += th) {
				for (int tx = x; tx < x + width; tx += tw) {
					int dw = std::min(tw, x + width  - tx);
					int dh = std::min(th, y + height - ty);
					SDL_Rect src  = { 0, 0, dw, dh };
					SDL_Rect dest = { tx, ty, dw, dh };
					SDL_RenderCopy(renderer, tex, &src, &dest);
				}
			}
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
