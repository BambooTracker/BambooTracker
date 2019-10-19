#ifndef PATTERN_POSITION_HPP
#define PATTERN_POSITION_HPP

struct PatternPosition
{
	int track, colInTrack, order, step;

	friend bool operator==(const PatternPosition& a, const PatternPosition& b);
	friend bool operator!=(const PatternPosition& a, const PatternPosition& b);

	void setCols(int track, int colInTrack);
	void setRows(int order, int step);

	int compareCols(const PatternPosition& b) const;
	int compareRows(const PatternPosition& b) const;

	static bool inRowRange(const PatternPosition& pos,
								   const PatternPosition& begin, const PatternPosition& last)
	{
		return (begin.compareRows(pos) <= 0 && pos.compareRows(last) <= 0);
	}

	static bool inRowRange(const PatternPosition& srcBegin, const PatternPosition& srcLast,
								 const PatternPosition& tgtBegin, const PatternPosition& tgtLast)
	{
		if (PatternPosition::inRowRange(srcBegin, tgtBegin, tgtLast)) return true;
		else if (PatternPosition::inRowRange(srcLast, tgtBegin, tgtLast)) return true;
		else return (srcBegin.compareRows(tgtBegin) < 0 && tgtLast.compareRows(srcLast));
	}
};

inline bool operator==(const PatternPosition& a, const PatternPosition& b)
{
	return ((a.track == b.track) && (a.colInTrack == b.colInTrack)
			&& (a.order == b.order) && (a.step == b.step));
}

inline bool operator!=(const PatternPosition& a, const PatternPosition& b)
{
	return !(a == b);
}

inline void PatternPosition::setCols(int track, int colInTrack)
{
	this->track = track;
	this->colInTrack = colInTrack;
}

inline void PatternPosition::setRows(int order, int step)
{
	this->order = order;
	this->step = step;
}

/// Return:
///		-2: this->track < b.track
///		-1: this->track == b.track && this->colInTrack < b.colInTrack
///		 0: this->track == b.track && this->colInTrack == b.colInTrack
///		 1: this->track == b.track && this->colInTrack > b.colInTrack
///		 2: this->track > b.track
inline int PatternPosition::compareCols(const PatternPosition& b) const
{
	if (track < b.track) return -2;
	else if (track > b.track) return 2;
	else {
		if (colInTrack < b.colInTrack) return -1;
		else if (colInTrack > b.colInTrack) return 1;
		else return 0;
	}
}

/// Return:
///		-2: this->order < b.order
///		-1: this->order == b.order && this->step < b.step
///		 0: *this == b
///		 1: this->order == b.order && this->step > b.step
///		 2: this->order > b.order
inline int PatternPosition::compareRows(const PatternPosition& b) const
{
	if (order < b.order) return -2;
	else if (order > b.order) return 2;
	else {
		if (step < b.step) return -1;
		else if (step > b.step) return 1;
		else return 0;
	}
}

#endif // PATTERN_POSITION_HPP
