#pragma once
#include <vector>
#include <memory>
#include <utility>
#include "abstract_instrument_property.hpp"
#include "sequence_iterator_interface.hpp"

struct CommandSequenceUnit
{
	int type;
	/// In SSG waveform and envelope,
	///		- If bit 17 is 0,
	///			* If bit 16 is 0, bit 0-15 is raw data
	///			* If bit 16 is 1, bit 0-7 is 2nd and bit 8-15 is 1st ratio value
	///		- If bit 17 is 1,
	///			* If bit 16 is 0, bit 0-15 is right shift value
	///			* If bit 16 is 1, bit 0-15 is left shift value
	int data;

	enum DataType : int
	{
		NODATA = -1,
		RAW,
		RATIO,
		LSHIFT,
		RSHIFT
	};

	inline static DataType checkDataType(int data)
	{
		if (data < 0) return DataType::NODATA;
		else if (0x20000 & data) return (0x10000 & data ? DataType::LSHIFT : DataType::RSHIFT);
		else if (0x10000 & data) return DataType::RATIO;
		else return DataType::RAW;
	}

	inline static int ratio2data(int first, int second)
	{
		return ((1 << 16) | (first << 8) | second);
	}

	inline static std::pair<int, int> data2ratio(int data)
	{
		return std::make_pair((data & 0x0000ff00) >> 8, data & 0x000000ff);
	}

	inline static int shift2data(int rshift)
	{
		if (rshift > 0) return ((2 << 16) | rshift);
		else return ((3 << 16) | -rshift);
	}

	/// Check whether data is left shift or right shift before call this method
	inline static int data2shift(int data)
	{
		return 0xffff & data;
	}
};

enum SequenceType : int
{
	NO_SEQUENCE_TYPE = -1,
	ABSOLUTE_SEQUENCE = 0,
	FIXED_SEQUENCE = 1,
	RELATIVE_SEQUENCE = 2
};

struct Loop
{
	int begin, end, times;
};

enum ReleaseType
{
	NoRelease,
	FixedRelease,
	AbsoluteRelease,
	RelativeRelease
};

struct Release
{
	ReleaseType type;
	int begin;
};

class CommandSequence : public AbstractInstrumentProperty
{
public:
	CommandSequence(int num, SequenceType seqType = SequenceType::NO_SEQUENCE_TYPE, int comType = 0, int comData = -1);
	CommandSequence(const CommandSequence& other);
	virtual ~CommandSequence() = default;
	std::unique_ptr<CommandSequence> clone();

	/// 0: Absolute
	/// 1: Fix
	/// 2: Relative
	void setType(SequenceType type);
	SequenceType getType() const;

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

	SequenceType type_;
	std::vector<CommandSequenceUnit> seq_;
	std::vector<Loop> loops_;
	Release release_;
};
