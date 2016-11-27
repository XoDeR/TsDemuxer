#pragma once
#include <cstdint>
#include <string>
#include <list>

namespace TsDemuxer
{
	inline uint8_t getUint8FromStream(const int8_t* p)
	{
		return *((uint8_t*)p);
	}

	inline uint16_t getUint16FromStream(const int8_t* p)
	{
		uint16_t n = ((uint8_t*)p)[0]; n <<= 8; n += ((uint8_t*)p)[1];
		return n;
	}

	inline uint32_t getUint32FromStream(const int8_t* p)
	{
		uint32_t n = ((uint8_t*)p)[0];
		n <<= 8; n += ((uint8_t*)p)[1];
		n <<= 8; n += ((uint8_t*)p)[2];
		n <<= 8; n += ((uint8_t*)p)[3];
		return n;
	}
	void getPrefixNameByFilename(const std::string& s, std::string& name);
	int scanDirectory(const char* path, std::list<std::string>& l);
	std::string trimSlash(const std::string& s);
}