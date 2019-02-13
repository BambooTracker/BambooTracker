#pragma once
#include <vector>
#include <memory>
#include "abstract_instrument_property.hpp"
#include "sequence_iterator_interface.hpp"

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

class CommandSequence : public AbstractInstrumentProperty
{
public:
	CommandSequence(int num, int seqType, int comType = 0, int comData = -1);
	CommandSequence(const CommandSequence& other);
	virtual ~CommandSequence() = default;
	std::unique_ptr<CommandSequence> clone();

	/// 0: Absolute
	/// 1: Fix
	/// 2: Relative
	void setType(int type);
	int getType() const;

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

	class Iterator : public SequenceIteratorInterface
	{
	public:
		explicit Iterator(CommandSequence* seq);
		int getPosition() const override;
		int getSequenceType() const override;
		int getCommandType() const override;
		int getCommandData() const override;
		int next(bool isReleaseBegin = false) override;
		int front() override;

	private:
		CommandSequence* seq_;
		int pos_;
		std::vector<Loop> loopStack_;
		bool isRelease_;
		float relReleaseRatio_;
	};

	std::unique_ptr<CommandSequence::Iterator> getIterator();

	bool isEdited() const;

private:
	const int DEF_COM_TYPE;
	const int DEF_COM_DATA;

	int type_;
	std::vector<CommandInSequence> seq_;
	std::vector<Loop> loops_;
	Release release_;
};
