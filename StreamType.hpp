#pragma once

namespace TsDemuxer
{
	namespace StreamType
	{
		enum
		{
			Data = 0,
			Mpeg2Video = 1,
			H264Video = 2,
			Vc1Video = 3,
			Ac3Audio = 4,
			Mpeg2Audio = 5,
			LpcmAudio = 6,
			DtsAudio = 7,
			AacAudio = 8,
			SvcVideo = 9,
			MvcVideo = 10,
			MjpegVideo = 11,
			HevcVideo = 12
		};
	}
} // namespace TsDemuxer