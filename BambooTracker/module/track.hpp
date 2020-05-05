#pragma once

#include <vector>
#include <unordered_set>
#include <memory>
#include <unordered_map>
#include "pattern.hpp"
#include "misc.hpp"

struct TrackAttribute;
struct OrderData;

class Track
{
public:
	Track(int number, SoundSource source, int channelInSource, int defPattenSize);
	Track(const Track& other);
	Track& operator=(const Track& other);
	Track(Track&& other) noexcept;
	Track& operator=(Track&& other) noexcept;

	void setAttribute(int number, SoundSource source, int channelInSource);
	TrackAttribute getAttribute() const;
	OrderData getOrderData(int order);
	size_t getOrderSize() const;
	Pattern& getPattern(int num);
	Pattern& getPatternFromOrderNumber(int num);
	int searchFirstUneditedUnusedPattern() const;
	int clonePattern(int num);
	std::vector<int> getEditedPatternIndices() const;
	std::unordered_set<int> getRegisteredInstruments() const;

	void registerPatternToOrder(int order, int pattern);
	void insertOrderBelow(int order);
	void deleteOrder(int order);
	void swapOrder(int a, int b);

	void changeDefaultPatternSize(size_t size);

	void setEffectDisplayWidth(size_t w);
	size_t getEffectDisplayWidth() const;

	void clearUnusedPatterns();
	void replaceDuplicateInstrumentsInPatterns(std::unordered_map<int, int> map);

	void transpose(int seminotes, std::vector<int> excludeInsts);

private:
	std::unique_ptr<TrackAttribute> attrib_;

	std::vector<int> order_;
	std::vector<Pattern> patterns_;
	size_t effetDisplayWidth_;
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
