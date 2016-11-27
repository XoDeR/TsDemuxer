#include "File.hpp"
#include "Common.hpp"

#ifndef O_LARGEFILE
#define O_LARGEFILE 0
#endif // O_LARGEFILE

TsDemuxer::File::~File()
{
	close();
}

bool TsDemuxer::File::open(int mode, const char* fmt, ...)
{
	filename.clear();

	char name[512];

	va_list ap;
	va_start(ap, fmt);
	vsprintf(name, fmt, ap);
	va_end(ap);

	int flags = 0;

	switch (mode)
	{
	case in:
		flags = O_LARGEFILE | O_BINARY | O_RDONLY;
		break;
	case out:
		flags = O_CREAT | O_TRUNC | O_LARGEFILE | O_BINARY | O_WRONLY;
		break;
	}

	fileDescriptor = ::_open(name, flags, 0644);
	if (fileDescriptor != -1)
	{
		filename = name;
		len = offset = 0;
		return true;
	}
	fprintf(stderr, "can`t open file %s\n", name);
	return false;
}

void TsDemuxer::File::close()
{
	if (fileDescriptor != -1)
	{
		flush();
		::_close(fileDescriptor);
		fileDescriptor = -1;
	}
	len = 0;
	offset = 0;
}

int TsDemuxer::File::flush(void)
{
	int l = 0;
	while (l < len)
	{
		int n = ::_write(fileDescriptor, buf + l, len - l);
		if (!n || n == -1)
		{
			break;
		}
		l += n;
	}
	len = 0;
	return l;
}

int TsDemuxer::File::write(const int8_t* p, int l)
{
	int rc = l;

	while (l > 0)
	{
		if (len >= maxBufferSize)
		{
			flush();
		}
		int n = maxBufferSize - len;
		if (n > l)
		{
			n = l;
		}
		memcpy(buf + len, p, n);
		len += n;
		p += n;
		l -= n;
	}
	return rc;
}
int TsDemuxer::File::read(int8_t* p, int l)
{
	const int8_t* tmp = p;
	while (l > 0)
	{
		int n = len - offset;
		if (n > 0)
		{
			if (n > l)
			{
				n = l;
			}
			memcpy(p, buf + offset, n);
			p += n;
			offset += n;
			l -= n;
		}
		else
		{
			int m = ::_read(fileDescriptor, buf, maxBufferSize);
			if (m == -1 || !m)
			{
				break;
			}
			len = m;
			offset = 0;
		}
	}

	return p - tmp;
}
