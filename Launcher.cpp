#include "Utilities.hpp"
#include "DeMuxer.hpp"
#include <list>
#include <string>

int main()
{
	std::list<std::string> playlist;
	std::list<std::string> l;
	TsDemuxer::scanDirectory(TsDemuxer::trimSlash("d:\\TsDemuxer.Test.0001").c_str(), l);
	l.sort();
	playlist.merge(l);
	for (const auto& s : playlist)
	{
		TsDemuxer::DeMuxer demuxer;
		demuxer.processFile(s.c_str());
	}
	return 0;
}
