#include "Demuxer.hpp"
#include "StreamType.hpp"
#include "File.hpp"
#include "Utilities.hpp"

bool TsDemuxer::DeMuxer::getIsTypeValid(uint8_t type)
{
	return strchr("\x01\x02\x80\x10\x1b\x24\xea\x1f\x20\x21\x03\x04\x11\x1c\x0f\x81\x06\x83\x84\x87\x82\x86\x8a\x85", type) ? true : false;
}

int TsDemuxer::DeMuxer::getStreamType(uint8_t type)
{
	switch (type)
	{
	case 0x01:
	case 0x02:
	case 0x10:
		return StreamType::Mpeg2Video;
	case 0x80:
		return hdmv ? StreamType::LpcmAudio : StreamType::Mpeg2Video;
	case 0x1b:
		return StreamType::H264Video;
	case 0x24:
		return StreamType::HevcVideo;
	case 0xea:
		return StreamType::Vc1Video;
	case 0x1f:
		return StreamType::SvcVideo;
	case 0x20:
		return StreamType::MvcVideo;
	case 0x21:
		return StreamType::MjpegVideo;
	case 0x81:
	case 0x06:
	case 0x83:
	case 0x84:
	case 0x87:
		return StreamType::Ac3Audio;
	case 0x03:
	case 0x04:
		return StreamType::Mpeg2Audio;
	case 0x82:
	case 0x86:
	case 0x8a:
	case 0x85:
		return StreamType::DtsAudio;
	case 0x11:
	case 0x1c:
	case 0x0f:
		return StreamType::AacAudio;
	}
	return StreamType::Data;
}

const char* TsDemuxer::DeMuxer::getFileExtensionByStreamType(uint8_t typeId)
{
	static const char* list[13] = { "sup", "m2v", "264", "vc1", "ac3", "m2a", "pcm", "dts", "aac", "svc", "mvc", "mjpg", "265" };
	if (typeId < 0 || typeId >= 13)
	{
		typeId = 0;
	}
	return list[typeId];
}

uint64_t TsDemuxer::DeMuxer::decodePts(const uint8_t* ptr)
{
	const uint8_t* p = (uint8_t*)ptr;
	uint64_t pts = ((p[0] & 0xe) << 29);
	pts |= ((p[1] & 0xff) << 22);
	pts |= ((p[2] & 0xfe) << 14);
	pts |= ((p[3] & 0xff) << 7);
	pts |= ((p[4] & 0xfe) >> 1);
	return pts;
}

int TsDemuxer::DeMuxer::demuxTsPacket(const int8_t* ptr)
{
	uint32_t timecode = 0;
	if (hdmv)
	{
		timecode = getUint32FromStream(ptr) & 0x3fffffff;
		ptr += 4;
	}
	const int8_t* endPtr = ptr + 188;
	if (ptr[0] != 0x47) // ts sync byte
	{
		return -1;
	}
	uint16_t pid = getUint16FromStream(ptr + 1);
	uint8_t flags = getUint8FromStream(ptr + 3);
	bool transportError = pid & 0x8000;
	bool payloadUnitStartIndicator = pid & 0x4000;
	bool adaptationFieldExist = flags & 0x20;
	bool payloadDataExist = flags & 0x10;
	pid &= 0x1fff;
	if (transportError)
	{
		return -2;
	}
	if (pid == 0x1fff || !payloadDataExist)
	{
		return 0;
	}
	// next
	ptr += 4;
	// skip adaptation field
	if (adaptationFieldExist)
	{
		ptr += getUint8FromStream(ptr) + 1;
		if (ptr >= endPtr)
		{
			return -3;
		}
	}
	Stream& s = streams[pid];
	if (!pid || (s.channel != 0xffff && s.type == 0xff))
	{
		// PSI
		if (payloadUnitStartIndicator)
		{
			// the beginning of the PSI table
			ptr++;
			if (ptr >= endPtr)
			{
				return -4;
			}
			if (*ptr != 0x00 && *ptr != 0x02)
			{
				return 0;
			}
			if (endPtr - ptr < 3)
			{
				return -5;
			}
			uint16_t l = getUint16FromStream(ptr + 1);
			if (l & 0x3000 != 0x3000)
			{
				return -6;
			}
			l &= 0x0fff;
			ptr += 3;
			int len = endPtr - ptr;
			if (l > len)
			{
				if (l > TsDemuxer::Table::maxBufferSize)
				{
					return -7;
				}
				s.psi.reset();
				memcpy(s.psi.buf, ptr, len);
				s.psi.offset += len;
				s.psi.len = l;
				return 0;
			}
			else
			{
				endPtr = ptr + l;
			}
		}
		else
		{
			// next part of PSI
			if (!s.psi.offset)
			{
				return -8;
			}
			int len = endPtr - ptr;
			if (len > TsDemuxer::Table::maxBufferSize - s.psi.offset)
			{
				return -9;
			}
			memcpy(s.psi.buf + s.psi.offset, ptr, len);
			s.psi.offset += len;
			if (s.psi.offset < s.psi.len)
			{
				return 0;
			}
			else
			{
				ptr = s.psi.buf;
				endPtr = ptr + s.psi.len;
			}
		}
		if (!pid)
		{
			// PAT
			ptr += 5;
			if (ptr >= endPtr)
			{
				return -10;
			}
			int len = endPtr - ptr - 4;
			if (len < 0 || len % 4)
			{
				return -11;
			}
			int n = len / 4;
			for (int i = 0; i < n; i++, ptr += 4)
			{
				uint16_t channel = getUint16FromStream(ptr);
				uint16_t pid = getUint16FromStream(ptr + 2);
				if (pid & 0xe000 != 0xe000)
				{
					return -12;
				}
				pid &= 0x1fff;
				if (!DeMuxer::channel || DeMuxer::channel == channel)
				{
					Stream& ss = streams[pid];
					ss.channel = channel;
					ss.type = 0xff;
				}
			}
		}
		else
		{
			// PMT
			ptr += 7;
			if (ptr >= endPtr)
			{
				return -13;
			}
			uint16_t infoLen = getUint16FromStream(ptr) & 0x0fff;
			ptr += infoLen + 2;
			endPtr -= 4;
			if (ptr >= endPtr)
			{
				return -14;
			}
			while (ptr < endPtr)
			{
				if (endPtr - ptr < 5)
				{
					return -15;
				}
				uint8_t type = getUint8FromStream(ptr);
				uint16_t pid = getUint16FromStream(ptr + 1);
				if (pid & 0xe000 != 0xe000)
				{
					return -16;
				}
				pid &= 0x1fff;
				infoLen = getUint16FromStream(ptr + 3) & 0x0fff;
				ptr += 5 + infoLen;
				// unknown streams are ignored
				if (getIsTypeValid(type) == true)
				{
					Stream& ss = streams[pid];
					if (ss.channel != s.channel || ss.type != type)
					{
						ss.channel = s.channel;
						ss.type = type;
						ss.id = ++s.id;

						if (!ss.file.getIsOpened())
						{
							if (!outputDirectory.empty())
							{
								char osSpecificSeparator = '\\';
#ifndef _WIN32
								osSpecificSeparator = '/';
#endif // _WIN32
								ss.file.open(File::out, "%s%c%sTrack.%i.%s", outputDirectory.c_str(), osSpecificSeparator, prefix.c_str(), pid, getFileExtensionByStreamType(getStreamType(ss.type)));
							}
							else
							{
								ss.file.open(File::out, "%sTrack.%i.%s", prefix.c_str(), pid, getFileExtensionByStreamType(getStreamType(ss.type)));
							}
						}
					}
				}
			}
			if (ptr != endPtr)
			{
				return -18;
			}
		}
	}
	else
	{
		if (s.type != 0xff)
		{
			// PES
			if (payloadUnitStartIndicator)
			{
				s.psi.reset();
				s.psi.len = 9;
			}
			while (s.psi.offset < s.psi.len)
			{
				int len = endPtr - ptr;
				if (len <= 0)
				{
					return 0;
				}
				int n = s.psi.len - s.psi.offset;
				if (len > n)
				{
					len = n;
				}
				memcpy(s.psi.buf + s.psi.offset, ptr, len);
				s.psi.offset += len;
				ptr += len;
				if (s.psi.len == 9)
				{
					s.psi.len += getUint8FromStream(s.psi.buf + 8);
				}
			}
			if (s.psi.len)
			{
				if (memcmp(s.psi.buf, "\x00\x00\x01", 3))
				{
					return -19;
				}
				s.streamId = getUint8FromStream(s.psi.buf + 3);
				uint8_t flags = getUint8FromStream(s.psi.buf + 7);
				s.frameCount++;
				switch (flags & 0xc0)
				{
				case 0x80: // PTS only
				{
					uint64_t pts = decodePts((uint8_t*)s.psi.buf + 9);
					if (s.dts > 0 && pts > s.dts)
					{
						s.frameLength = pts - s.dts;
					}
					s.dts = pts;
					if (pts > s.lastPts)
					{
						s.lastPts = pts;
					}
					if (!s.firstPts)
					{
						s.firstPts = pts;
					}
				}
				break;
				case 0xc0: // PTS, DTS
				{
					uint64_t pts = decodePts((uint8_t*)s.psi.buf + 9);
					uint64_t dts = decodePts((uint8_t*)s.psi.buf + 14);
					if (s.dts > 0 && dts > s.dts)
					{
						s.frameLength = dts - s.dts;
					}
					s.dts = dts;
					if (pts > s.lastPts)
					{
						s.lastPts = pts;
					}
					if (!s.firstDts)
					{
						s.firstDts = dts;
					}
				}
				break;
				}
				s.psi.reset();
			}
			if (s.frameCount)
			{
				int len = endPtr - ptr;
				if (s.file.getIsOpened())
				{
					s.file.write(ptr, len);
				}
			}
		}
	}
	return 0;
}

int TsDemuxer::DeMuxer::processFile(const char* name)
{
	prefix.clear();
	int8_t buf[192];
	int bufLen = 0;
	TsDemuxer::File file;
	if (!file.open(File::in, "%s", name))
	{
		fprintf(stderr, "Can`t open file %s\n", name);
		return -1;
	}
	getPrefixNameByFilename(name, prefix);
	if (!prefix.empty())
	{
		prefix += '.';
	}
	for (uint64_t pn = 1;; pn++)
	{
		if (bufLen)
		{
			if (file.read(buf, bufLen) != bufLen)
			{
				break;
			}
		}
		else
		{
			if (file.read(buf, 188) != 188)
			{
				break;
			}
			if (buf[0] == 0x47 && buf[4] != 0x47)
			{
				bufLen = 188;
				fprintf(stderr, "TS stream detected in %s (packet length=%i)\n", name, bufLen);
				hdmv = false;
			}
			else if (buf[0] != 0x47 && buf[4] == 0x47)
			{
				if (file.read(buf + 188, 4) != 4)
				{
					break;
				}
				bufLen = 192;
				fprintf(stderr, "M2TS stream detected in %s (packet length=%i)\n", name, bufLen);
				hdmv = true;
			}
			else
			{
				fprintf(stderr, "Unknown stream type in %s\n", name);
				return -1;
			}
		}

		int errorCode = demuxTsPacket(buf);
		if (errorCode != 0)
		{
			fprintf(stderr, "%s: invalid packet %llu (%i)\n", name, pn, errorCode);
			return -1;
		}
	}
	return 0;
}