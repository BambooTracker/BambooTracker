#ifndef PATTERN_POSITION_HPP
#define PATTERN_POSITION_HPP

struct PatternPosition
{
	int track, colInTrack, order, step;

	friend bool operator==(const PatternPosition& a, const PatternPosition& b);
	friend bool operator!=(const PatternPosition& a, const PatternPosition& b);

	void setCols(int track, int colInTrack);
	void setRows(int order, int step);
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

#endif // PATTERN_POSITION_HPP
