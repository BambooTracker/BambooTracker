#pragma once
#include <vector>
#include <memory>
#include <utility>
#include "abstract_instrument_property.hpp"
#include "sequence_iterator_interface.hpp"

struct CommandSequenceUnit
{
	int type;
	/// In SSG waveform and envelope, when bit 16 is ratio flag
	int data;

	inline static bool isRatioData(int data)
	{
		return (data > -1 && data & 0x00010000);
	}

	inline static int ratio2data(int first, int second)
	{
		return ((1 << 16) | (first << 8) | second);
	}

	inline static std::pair<int, int> data2ratio(int data)
	{
		return std::pair<int, int>((data & 0x0000ff00) >> 8, data & 0x000000ff);
	}
};

struct Loop
{
	int begin, end, times;
};

enum ReleaseType
{
	NO_RELEASE,
	FIXED,
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
	std::vector<CommandSequenceUnit> getSequence() const;
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
		int end() override;

	private:
		CommandSequence* seq_;
		int pos_;
		bool started_;
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
	std::vector<CommandSequenceUnit> seq_;
	std::vector<Loop> loops_;
	Release release_;
};
