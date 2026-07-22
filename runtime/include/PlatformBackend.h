#pragma once

#include <string>
#include <cstdint>
#include <unordered_map>
#include <vector>
#include "lz4.h"
#include "PakFile.h"
#include "EndiannessHelper.h"

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

	//Pak file stuff, maybe move to application class? - shish
	virtual bool PakFileEntryExists(std::string entry) { return pakFile.Exists(entry); }
	virtual std::vector<uint8_t> GetPakFileEntryData(std::string entry) { return pakFile.GetData(entry); }

	PakFile& GetPakFile() { return pakFile; }
	std::vector<uint8_t> DecompressCollisionMaskLZ4(const std::vector<uint8_t>& data)
	{
		if (data.size() < 12)
			return {};

		const uint8_t* ptr = data.data();


		uint16_t width = EndiannessHelper::ReadLE16(ptr);
		uint16_t height = EndiannessHelper::ReadLE16(ptr);
		uint32_t uncompressedSize = EndiannessHelper::ReadLE32(ptr);
		uint32_t compressedSize = EndiannessHelper::ReadLE32(ptr);

		(void)width;
		(void)height;



		if (width == 0 || height == 0 || compressedSize == 0 || uncompressedSize == 0)
			return {};

		uint32_t expectedSize = ((width + 7) / 8) * height;

		if (uncompressedSize != expectedSize)
			return {};

		if (data.size() < 12u + compressedSize)
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

	const std::vector<uint8_t>* GetCollisionMaskData(unsigned int imageId)
	{
		auto it = collisionMaskCache.find(imageId);
		if (it != collisionMaskCache.end())
			return &it->second;

		std::string basePath = "images/masks/" + std::to_string(imageId);

		std::vector<uint8_t> data = pakFile.GetData(basePath + ".bin.lz4");

		if (!data.empty())
		{
			std::vector<uint8_t> decompressed = DecompressCollisionMaskLZ4(data);
			if (decompressed.empty())
				return nullptr;

			it = collisionMaskCache.emplace(imageId, std::move(decompressed)).first;
			return &it->second;
		}

		data = pakFile.GetData(basePath + ".bin");

		if (data.empty())
			return nullptr;

		it = collisionMaskCache.emplace(imageId, std::move(data)).first;
		return &it->second;
	}

	virtual void Log(std::string text) {}
protected:
	PakFile pakFile;
	std::unordered_map<unsigned int, std::vector<uint8_t>> collisionMaskCache;
}; 