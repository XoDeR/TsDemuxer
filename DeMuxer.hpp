#pragma once
#include <cstdint>
#include <map>
#include <string>

#include "Stream.hpp"

namespace TsDemuxer
{
	class DeMuxer
	{
	public:
		std::map<uint16_t, Stream> streams;
		bool hdmv = false; // HDMV mode, 192 bytes packets
		int channel = 0; // channel for demux
		std::string prefix; // output file name prefix (autodetect)
		std::string outputDirectory;
	private:
		bool getIsTypeValid(uint8_t type);
		uint64_t decodePts(const uint8_t* ptr);
		int getStreamType(uint8_t type);
		const char* getFileExtensionByStreamType(uint8_t typeId);
		// take 188/192 bytes TS/M2TS packet
		int demuxTsPacket(const int8_t* ptr);
	public:
		int processFile(const char* name);
		void reset()
		{
			for (auto& pair : streams)
			{
				pair.second.reset();
			}
		}
	};
} // namespace TsDemuxer