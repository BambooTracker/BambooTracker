#include "command_sequence.hpp"

CommandSequence::CommandSequence(int num, int type, int data)
	: AbstructInstrumentProperty(num),
	  release_{ ReleaseType::NO_RELEASE, -1 }
{
	seq_.push_back({ type, data });
}

CommandSequence::CommandSequence(const CommandSequence& other)
	: AbstructInstrumentProperty(other)
{
	seq_ = other.seq_;
	loops_ = other.loops_;
	release_ = other.release_;
}

std::unique_ptr<CommandSequence> CommandSequence::clone()
{
	return std::unique_ptr<CommandSequence>(std::make_unique<CommandSequence>(*this));
}

size_t CommandSequence::getSequenceSize() const
{
	return seq_.size();
}

int CommandSequence::getSequenceTypeAt(int n)
{
	return seq_.at(n).type;
}

int CommandSequence::getSequenceDataAt(int n)
{
	return seq_.at(n).data;
}

std::vector<CommandInSequence> CommandSequence::getSequence() const
{
	return seq_;
}

void CommandSequence::addSequenceCommand(int type, int data)
{
	seq_.push_back({ type, data });
}

void CommandSequence::removeSequenceCommand()
{
	seq_.pop_back();

	// Modify loop
	if (!loops_.empty()) {
		if (seq_.size() == loops_.back().begin) {
			loops_.pop_back();
		}
		else if (seq_.size() == loops_.back().end) {
			--loops_.back().end;
		}
	}

	// Modify release
	if (release_.begin == seq_.size())
		release_.begin = -1;
}

void CommandSequence::setSequenceCommand(int n, int type, int data)
{
	seq_.at(n) = { type, data };
}

size_t CommandSequence::getNumberOfLoops() const
{
	return loops_.size();
}

int CommandSequence::getBeginningCountOfLoop(int n)
{
	return loops_.at(n).begin;
}

int CommandSequence::getEndCountOfLoop(int n)
{
	return loops_.at(n).end;
}

int CommandSequence::getTimesOfLoop(int n)
{
	return loops_.at(n).times;
}

std::vector<Loop> CommandSequence::getLoops() const
{
	return loops_;
}

void CommandSequence::setLoops(std::vector<int> begins, std::vector<int> ends, std::vector<int> times)
{
	loops_.clear();
	for (size_t i = 0; i < begins.size(); ++i) {
		loops_.push_back({ begins.at(i), ends.at(i), times.at(i) });
	}
}

int CommandSequence::getReleaseBeginningCount() const
{
	return release_.begin;
}

ReleaseType CommandSequence::getReleaseType() const
{
	return release_.type;
}

Release CommandSequence::getRelease() const
{
	return release_;
}

void CommandSequence::setRelease(ReleaseType type, int begin)
{
	release_ = { type, begin };
}

std::unique_ptr<CommandSequence::Iterator> CommandSequence::getIterator()
{
	return std::unique_ptr<Iterator>(std::make_unique<Iterator>(this));
}

/****************************************/
CommandSequence::Iterator::Iterator(CommandSequence* seq)
	: seq_(seq),
	  pos_(0),
	  isRelease_(false)
{
}

int CommandSequence::Iterator::getPosition() const
{
	return pos_;
}

int CommandSequence::Iterator::getCommandType() const
{
	return (pos_ == -1) ? -1 : seq_->getSequenceTypeAt(pos_);
}

int CommandSequence::Iterator::getCommandData() const
{
	return (pos_ == -1) ? -1 : seq_->getSequenceDataAt(pos_);
}

int CommandSequence::Iterator::next(bool isReleaseBegin)
{
	if (!isReleaseBegin && pos_ == -1) return -1;

	int next;
	if (isReleaseBegin) {
		loopStack_.clear();
		isRelease_ = true;
		next = seq_->release_.begin;
	}
	else {
		next = pos_ + 1;
	}

	if (loopStack_.empty()) {
		for (auto& l : seq_->loops_) {
			if (next < l.begin) break;
			else if (next == l.begin) {
				loopStack_.push_back({ l.begin, l.end, (l.times == 1) ? -1 : l.times - 1});
			}
		}
		pos_ = next;
	}
	else {
		if (pos_ == loopStack_.back().end) {
			if (loopStack_.back().times < 0) {	// Infinity loop
				pos_ = loopStack_.back().begin;
			}
			else {
				if (loopStack_.back().times) {
					pos_ = loopStack_.back().begin;
					--loopStack_.back().times;
				}
				else {
					pos_ = next;
					loopStack_.pop_back();
				}
			}
		}
		else {
			pos_ = next;
		}
	}

	if (!isRelease_ && pos_ == seq_->release_.begin) {
		pos_ = -1;
	}
	else if (pos_ == seq_->seq_.size()) {
		pos_ = -1;
	}

	return pos_;
}

int CommandSequence::Iterator::front()
{
	loopStack_.clear();
	isRelease_ = false;
	pos_ = 0;
	return pos_;
}
