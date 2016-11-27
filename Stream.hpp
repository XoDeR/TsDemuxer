#pragma once
#include <cstdint>
#include "Table.hpp"
#include "File.hpp"

namespace TsDemuxer
{
	class Stream
	{
	public:
		uint16_t channel = 0xffff;
		uint8_t id = 0; // stream number in channel
		uint8_t type = 0xff;
		Table psi; // PAT, PMT cache (for PSI streams only)
		uint8_t streamId = 0;
		File file; // output file
		uint64_t dts = 0; // MPEG stream DTS (presentation time for audio, decode time for video)
		uint64_t firstDts = 0;
		uint64_t firstPts = 0;
		uint64_t lastPts = 0;
		uint32_t frameLength = 0; // frame length in ticks (90 ticks == 1 ms, 90000/frame_length == fps)
		uint64_t frameCount = 0;
		void reset()
		{
			psi.reset();
			dts = 0;
			firstPts = 0;
			lastPts = 0;
			frameLength = 0;
			frameCount = 0;
		}
	};
} // namespace TsDemuxer