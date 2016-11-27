#include "Utilities.hpp"
#include "Common.hpp"

void TsDemuxer::getPrefixNameByFilename(const std::string& s, std::string& name)
{
	int ll = s.length();
	const char* p = s.c_str();
	while (ll > 0)
	{
		if (p[ll - 1] == '/' || p[ll - 1] == '\\')
		{
			break;
		}
		ll--;
	}
	p += ll;
	int cn = 0;
	const char* pp = strchr(p, '.');
	if (pp != nullptr)
	{
		name.assign(p, pp - p);
	}
}

std::string TsDemuxer::trimSlash(const std::string & s)
{
	const char* p = s.c_str() + s.length();
	while (p > s.c_str() && (p[-1] == '/' || p[-1] == '\\'))
	{
		p--;
	}
	return s.substr(0, p - s.c_str());
}

#ifdef _WIN32
int TsDemuxer::scanDirectory(const char* path, std::list<std::string>& l)
{
	_finddata_t fileinfo;
	intptr_t dir = _findfirst((std::string(path) + "\\*.*").c_str(), &fileinfo);
	if (dir == -1)
	{
		perror(path);
	}
	else
	{
		while (!_findnext(dir, &fileinfo))
		{
			if (!(fileinfo.attrib&_A_SUBDIR) && *fileinfo.name != '.')
			{
				char p[512];
				int n = sprintf(p, "%s\\%s", path, fileinfo.name);
				l.push_back(std::string(p, n));
			}
		}
	}
	_findclose(dir);
	return l.size();
}
#else
int TsDemuxer::scanDirectory(const char* path, std::list<std::string>& l)
{
	DIR* dir = opendir(path);
	if (!dir)
	{
		perror(path);
	}
	else
	{
		dirent* d;
		while ((d = readdir(dir)))
		{
			if (d->d_type == DT_UNKNOWN)
			{
				char p[512];
				if (snprintf(p, sizeof(p), "%s/%s", path, d->d_name) > 0)
				{
					struct stat st;
					if (stat(p, &st) != -1)
					{
						if (S_ISREG(st.st_mode))
						{
							d->d_type = DT_REG;
						}
						else if (S_ISLNK(st.st_mode))
						{
							d->d_type = DT_LNK;
						}
					}
				}
			}
			if (*d->d_name != '.' && (d->d_type == DT_REG || d->d_type == DT_LNK))
			{
				char p[512];
				int n = sprintf(p, "%s/%s", path, d->d_name);
				l.push_back(std::string(p, n));
			}
		}
		closedir(dir);
	}
	return l.size();
}
#endif // _WIN32