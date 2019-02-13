#pragma once

#include <vector>
#include <set>
#include <memory>
#include "pattern.hpp"
#include "misc.hpp"

struct TrackAttribute;
struct OrderData;

class Track
{
public:
	Track(int number, SoundSource source, int channelInSource, int defPattenSize);
	Track(const Track& other);
	TrackAttribute getAttribute() const;
	OrderData getOrderData(int order);
	size_t getOrderSize() const;
	Pattern& getPattern(int num);
	Pattern& getPatternFromOrderNumber(int num);
	int searchFirstUneditedUnusedPattern() const;
	int clonePattern(int num);
	std::vector<int> getEditedPatternIndices() const;
	std::set<int> getRegisteredInstruments() const;

	void registerPatternToOrder(int order, int pattern);
	void insertOrderBelow(int order);
	void deleteOrder(int order);
	void swapOrder(int a, int b);

	void changeDefaultPatternSize(size_t size);

	void clearUnusedPatterns();

private:
	std::unique_ptr<TrackAttribute> attrib_;

	std::vector<int> order_;
	std::vector<Pattern> patterns_;
};

struct TrackAttribute
{
	int number;
	SoundSource source;
	int channelInSource;
};


struct OrderData
{
	TrackAttribute trackAttribute;
	int order;
	int patten;
};
