#pragma once
#include <cstdint>
#include <string>

namespace TsDemuxer
{
	class File
	{
	private:
		int fileDescriptor = -1;
		enum
		{
			maxBufferSize = 2048
		};
		int8_t buf[maxBufferSize];
		int len = 0;
		int offset = 0;
	public:
		std::string filename;
		enum
		{
			in = 0,
			out = 1
		};

		~File();
		bool open(int mode, const char* fmt, ...);
		void close();
		int write(const int8_t* p, int l);
		int flush();
		int read(int8_t* p, int l);
		bool getIsOpened()
		{
			return fileDescriptor == -1 ? false : true;
		}
	};
} // namespace TsDemuxer