#pragma once
#include <cstdint>

namespace TsDemuxer
{
	class Table
	{
	public:
		enum
		{
			maxBufferSize = 512
		};
		int8_t buf[maxBufferSize];
		uint16_t len = 0;
		uint16_t offset = 0;
		void reset()
		{
			offset = 0;
			len = 0;
		}
	};
} // namespace TsDemuxer