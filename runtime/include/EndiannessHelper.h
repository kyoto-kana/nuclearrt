#pragma once

#include <cstdint>

class EndiannessHelper
{
public:
	static uint16_t ReadLE16(const uint8_t*& p)
	{
		uint16_t v =
			(uint16_t)p[0] |
			((uint16_t)p[1] << 8);

		p += 2;
		return v;
	}

	static uint32_t ReadLE32(const uint8_t*& p)
	{
		uint32_t v =
			(uint32_t)p[0] |
			((uint32_t)p[1] << 8) |
			((uint32_t)p[2] << 16) |
			((uint32_t)p[3] << 24);

		p += 4;
		return v;
	}

	static int32_t ReadLEInt32(const uint8_t*& p)
	{
		return (int32_t)ReadLE32(p);
	}
};