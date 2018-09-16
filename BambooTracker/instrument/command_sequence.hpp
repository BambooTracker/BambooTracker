#pragma once
#include <vector>
#include <memory>
#include "abstruct_instrument_property.hpp"

struct CommandInSequence
{
	int type, data;
};

struct Loop
{
	int begin, end, times;
};

enum ReleaseType
{
	NO_RELEASE,
	FIX,
	ABSOLUTE,
	RELATIVE
};

struct Release
{
	ReleaseType type;
	int begin;
};

class CommandSequence : public AbstructInstrumentProperty
{
public:
	CommandSequence(int num, int type = 0, int data = -1);
	CommandSequence(const CommandSequence& other);
	virtual ~CommandSequence() = default;
	std::unique_ptr<CommandSequence> clone();

	size_t getSequenceSize() const;
	int getSequenceTypeAt(int n);
	int getSequenceDataAt(int n);
	std::vector<CommandInSequence> getSequence() const;
	void addSequenceCommand(int type, int data);
	void removeSequenceCommand();
	void setSequenceCommand(int n, int type, int data);

	size_t getNumberOfLoops() const;
	int getBeginningCountOfLoop(int n);
	int getEndCountOfLoop(int n);
	int getTimesOfLoop(int n);
	std::vector<Loop> getLoops() const;
	void setLoops(std::vector<int> begins, std::vector<int> ends, std::vector<int> times);

	int getReleaseBeginningCount() const;
	ReleaseType getReleaseType() const;
	Release getRelease() const;
	void setRelease(ReleaseType type, int begin);

	class Iterator
	{
	public:
		explicit Iterator(CommandSequence* seq);
		int getPosition() const;
		int getCommandType() const;
		int getCommandData() const;
		int next(bool isReleaseBegin = false);
		int front();

	private:
		CommandSequence* seq_;
		int pos_;
		std::vector<Loop> loopStack_;
		bool isRelease_;
		float relReleaseRatio_;
	};

	std::unique_ptr<CommandSequence::Iterator> getIterator();

private:
	std::vector<CommandInSequence> seq_;
	std::vector<Loop> loops_;
	Release release_;
};
