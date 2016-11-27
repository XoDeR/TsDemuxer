#pragma once

#ifdef _WIN32
#include <windows.h>
#endif // _WIN32

#include <sys/types.h>
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#include <cstdint>
#else // POSIX
#include <dirent.h>
#include <unistd.h>
#define O_BINARY 0
#endif // _WIN32