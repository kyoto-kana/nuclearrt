#pragma once

#include <string>
#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <vector>
#include "CollisionInstanceBounds.h"
#include "PakFile.h"
#include "lz4.h"
//TODO: Move to somewhere better
struct Vector2D
{
	float x;
	float y;
    Vector2D& operator*=(const Vector2D& other) {
        x *= other.x;
        y *= other.y;
        return *this;
    }
    Vector2D& operator/=(const Vector2D& other) {
        x /= other.x;
        y /= other.y;
        return *this;
    }
    Vector2D& operator+=(const Vector2D& other) {
        x += other.x;
        y += other.y;
        return *this;
    }
    Vector2D& operator-=(const Vector2D& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }
};
struct Polygon
{
	std::vector<Vector2D> vertices;
	//std::vector<uint32_t> indicies;
};
struct PolygonCollisionData
{
	std::vector<Polygon> Polygons;
};

static inline Vector2D RotateVertex(float cx, float cy, float sinAngle, float cosAngle, Vector2D p)
{
  float s = sinAngle;
  float c = cosAngle;

  // translate point back to origin:
  p.x -= cx;
  p.y -= cy;

  // rotate point
  float xnew = p.x * c - p.y * s;
  float ynew = p.x * s + p.y * c;

  // translate point back:
  p.x = xnew + cx;
  p.y = ynew + cy;

  return p;
}

static inline Vector2D ConvertVertexToWorldSpace(Vector2D pos, const CollisionInstanceBounds& bounds, float sinAngle, float cosAngle)
{
	// Scale
	pos *= Vector2D{bounds.scaleX,bounds.scaleY};
	// hotspot offset
	pos -= Vector2D{static_cast<float>(bounds.hotspotX),static_cast<float>(bounds.hotspotY)};
	// Rotate
	pos = RotateVertex(0,0, sinAngle, cosAngle, pos);
	// Transform
	pos += Vector2D{static_cast<float>(bounds.centerX - bounds.scrollX),static_cast<float>(bounds.centerY - bounds.scrollY)};

	return pos;
}

static inline Vector2D ConvertVertexToLocalSpace(Vector2D pos, const CollisionInstanceBounds& bounds, float sinAngle, float cosAngle)
{
	// Transform
	pos -= Vector2D{static_cast<float>(bounds.centerX - bounds.scrollX),static_cast<float>(bounds.centerY - bounds.scrollY)};
	// Rotate
	pos = RotateVertex(0,0, -sinAngle, cosAngle, pos);
	// hotspot offset
	pos += Vector2D{static_cast<float>(bounds.hotspotX),static_cast<float>(bounds.hotspotY)};
	// Scale
	pos /= Vector2D{bounds.scaleX,bounds.scaleY};

	return pos;
}



class PlatformBackend {
public:
	virtual void Initialize() {}
	virtual void Deinitialize() {}

	virtual std::string GetName() const { return ""; }

	virtual bool ShouldQuit() { return false; }

	virtual std::string GetPlatformName() { return "Unknown"; }
	virtual std::string GetAssetsDirectory() { return ""; }

	virtual std::string GetAppDrive() { return ""; }
	virtual std::string GetAppDirectory() { return ""; }
	virtual std::string GetAppPath() { return ""; }

	virtual unsigned int GetTicks() { return 0; }
	virtual float GetTimeDelta() { return 0.0f; }
	virtual void Delay(unsigned int ms) {}

	virtual void SetFullscreen(bool fullscreen) {}

	//Pak file stuff, maybe move to application class? - shish
	virtual bool PakFileEntryExists(std::string entry) { return pakFile.Exists(entry); }
	virtual std::vector<uint8_t> GetPakFileEntryData(std::string entry) { return pakFile.GetData(entry); }

	PakFile& GetPakFile() { return pakFile; }

	
	PolygonCollisionData DecompressPolygonData(const std::vector<uint8_t>& data)
	{
		if (data.size() < 4)
		{
			Log("Error while loading polygon data: File too small!");
			return {};
		}

		const uint8_t* ptr = data.data();

		uint32_t uncompressedSize, compressedSize;

		std::memcpy(&uncompressedSize, ptr, 4); ptr += 4;

		compressedSize = static_cast<uint32_t>(data.size() - 4);

		if (compressedSize == 0 || uncompressedSize == 0)
		{
			Log("Error while loading polygon data: Something went wrong while deserializing!");
			return {};
		}

		std::vector<uint8_t> output(uncompressedSize);

		int result = LZ4_decompress_safe(
			reinterpret_cast<const char*>(ptr),
			reinterpret_cast<char*>(output.data()),
			(int)compressedSize,
			(int)uncompressedSize
		);

		if (result < 0 || (uint32_t)result != uncompressedSize)
		{
			Log("Error while loading polygon data: Something went wrong while decompressing!");
			return {};
		}

		const uint8_t* deptr = reinterpret_cast<const uint8_t*>(output.data());
		PolygonCollisionData polygonCollisionData;
		uint32_t numPolygons;
		std::memcpy(&numPolygons, deptr, 4); deptr += 4;

		for(int i = 0; i < numPolygons; i++)
		{
			uint32_t pointCounts;
			std::memcpy(&pointCounts, deptr, 4); deptr += 4;
			Polygon poly;
			for(int i = 0; i < pointCounts; i++)
			{
				uint32_t x, y;
				std::memcpy(&x, deptr, 4); deptr += 4;
				std::memcpy(&y, deptr, 4); deptr += 4;
				Vector2D vertexPositions = { static_cast<float>(x),  static_cast<float>(y)};
				poly.vertices.push_back(vertexPositions);
			}/*
			uint32_t indiciesCount;
			std::memcpy(&indiciesCount, deptr, 4); deptr += 4;
			for(int i = 0; i < indiciesCount; i++)
			{
				uint32_t index;
				std::memcpy(&index, deptr, 4); deptr += 4;
				poly.indicies.push_back(index);
			}*/
			polygonCollisionData.Polygons.push_back(poly);
		}

		return polygonCollisionData;
	}

	std::vector<uint8_t> DecompressCollisionMask(const std::vector<uint8_t>& data)
	{
		if (data.size() < 8)
			return {};

		const uint8_t* ptr = data.data();

		uint16_t width, height;
		uint32_t uncompressedSize, compressedSize;

		std::memcpy(&width,            ptr, 2); ptr += 2;
		std::memcpy(&height,           ptr, 2); ptr += 2;
		std::memcpy(&uncompressedSize, ptr, 4); ptr += 4;
		//std::memcpy(&compressedSize,   ptr, 4); ptr += 4;

		compressedSize = static_cast<uint32_t>(data.size() - 8);

		if (width == 0 || height == 0 || compressedSize == 0 || uncompressedSize == 0)
			return {};

		if ((uint64_t)((width + 7) / 8) * height != uncompressedSize)
			return {};

		// don't load weirdly large masks
		if (uncompressedSize > (8 * 1024 * 1024))
			return {};

		std::vector<uint8_t> output(uncompressedSize);

		int result = LZ4_decompress_safe(
			reinterpret_cast<const char*>(ptr),
			reinterpret_cast<char*>(output.data()),
			(int)compressedSize,
			(int)uncompressedSize
		);

		if (result < 0 || (uint32_t)result != uncompressedSize)
			return {};

		return output;
	}
	
	const PolygonCollisionData* GetCollisionPolygonData(unsigned int imageId) {
		auto it = collisionPolygonCache.find(imageId);
		if (it != collisionPolygonCache.end())
			return &it->second;
		std::vector<uint8_t> data = pakFile.GetData("images/masks/" + std::to_string(imageId) + ".bin");
		if (!data.empty())
		{
			PolygonCollisionData decompressed = DecompressPolygonData(data);
			/*if (decompressed.empty())
				return nullptr;*/

			it = collisionPolygonCache.emplace(imageId, std::move(decompressed)).first;
			return &it->second;
		}

		return nullptr;
	}

	const std::vector<uint8_t>* GetCollisionMaskData(unsigned int imageId) {
		auto it = collisionMaskCache.find(imageId);
		if (it != collisionMaskCache.end())
			return &it->second;
		std::vector<uint8_t> data = pakFile.GetData("images/masks/" + std::to_string(imageId) + ".bin");
		if (!data.empty())
		{
			std::vector<uint8_t> decompressed = DecompressCollisionMask(data);
			if (decompressed.empty())
				return nullptr;

			it = collisionMaskCache.emplace(imageId, std::move(decompressed)).first;
			return &it->second;
		}

		return nullptr;
	}

	virtual void Log(std::string text) {}
protected:
	PakFile pakFile;
	std::unordered_map<unsigned int, std::vector<uint8_t>> collisionMaskCache;
	std::unordered_map<unsigned int, PolygonCollisionData> collisionPolygonCache;
}; 