#ifndef TRACK_VISIBILITY_MEMORY_HANDLER_HPP
#define TRACK_VISIBILITY_MEMORY_HANDLER_HPP

#include <vector>
#include <QString>
#include "misc.hpp"

class TrackVisibilityMemoryHandler
{
public:
	static bool saveTrackVisibilityMemory(const SongType type, const std::vector<int>& visTracks);
	static bool loadTrackVisibilityMemory(SongType& type, std::vector<int>& visTracks);

private:
	const static QString ORGANIZATION_;
	const static QString FILE_;

	TrackVisibilityMemoryHandler();
};

#endif // TRACK_VISIBILITY_MEMORY_HANDLER_HPP
