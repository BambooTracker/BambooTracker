#include "command_sequence.hpp"

CommandSequence::CommandSequence(int num, SequenceType seqType, int comType, int comData)
	: AbstractInstrumentProperty(num),
	  DEF_COM_TYPE(comType),
	  DEF_COM_DATA(comData),
	  type_(seqType),
	  release_{ ReleaseType::NoRelease, -1 }
{
	seq_.push_back({ comType, comData });
}

CommandSequence::CommandSequence(const CommandSequence& other)
	: AbstractInstrumentProperty(other),
	  DEF_COM_TYPE(other.DEF_COM_TYPE),
	  DEF_COM_DATA(other.DEF_COM_DATA),
	  type_(other.type_),
	  seq_(other.seq_),
	  loops_(other.loops_),
	  release_(other.release_)
{
}

bool operator==(const CommandSequence& a, const CommandSequence& b)
{
	return (a.type_ == b.type_ && a.seq_ == b.seq_ && a.loops_ == b.loops_ && a.release_ == b.release_);
}

std::unique_ptr<CommandSequence> CommandSequence::clone()
{
	std::unique_ptr<CommandSequence> clone = std::make_unique<CommandSequence>(*this);
	clone->clearUserInstruments();
	return clone;
}

void CommandSequence::setType(SequenceType type)
{
	type_ = type;
}

SequenceType CommandSequence::getType() const
{
	return type_;
}

size_t CommandSequence::getSequenceSize() const
{
	return seq_.size();
}

int CommandSequence::getSequenceTypeAt(int n)
{
	return seq_.at(static_cast<size_t>(n)).type;
}

int CommandSequence::getSequenceDataAt(int n)
{
	return seq_.at(static_cast<size_t>(n)).data;
}

std::vector<CommandSequenceUnit> CommandSequence::getSequence() const
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
		if (static_cast<int>(seq_.size()) == loops_.back().begin) {
			loops_.pop_back();
		}
		else if (static_cast<int>(seq_.size()) == loops_.back().end) {
			--loops_.back().end;
		}
	}

	// Modify release
	if (release_.begin == static_cast<int>(seq_.size()))
		release_.begin = -1;
}

void CommandSequence::setSequenceCommand(int n, int type, int data)
{
	seq_.at(static_cast<size_t>(n)) = { type, data };
}

size_t CommandSequence::getNumberOfLoops() const
{
	return loops_.size();
}

int CommandSequence::getBeginningCountOfLoop(int n)
{
	return loops_.at(static_cast<size_t>(n)).begin;
}

int CommandSequence::getEndCountOfLoop(int n)
{
	return loops_.at(static_cast<size_t>(n)).end;
}

int CommandSequence::getTimesOfLoop(int n)
{
	return loops_.at(static_cast<size_t>(n)).times;
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

bool CommandSequence::isEdited() const
{
	return  (seq_.size() != 1 || seq_.front().type != DEF_COM_TYPE || seq_.front().data != DEF_COM_DATA
			|| loops_.size() || release_.begin > -1);
}

/****************************************/
CommandSequence::Iterator::Iterator(CommandSequence* seq)
	: seq_(seq),
	  pos_(0),
	  started_(false),
	  isRelease_(false),
	  relReleaseRatio_(1)
{
}

int CommandSequence::Iterator::getPosition() const
{
	return pos_;
}

int CommandSequence::Iterator::getSequenceType() const
{
	return seq_->type_;
}

int CommandSequence::Iterator::getCommandType() const
{
	return (pos_ == -1)
			? -1
			: isRelease_
			  ? static_cast<int>(seq_->getSequenceTypeAt(pos_) * relReleaseRatio_)
			  : seq_->getSequenceTypeAt(pos_);
}

int CommandSequence::Iterator::getCommandData() const
{
	return (pos_ == -1) ? -1 : seq_->getSequenceDataAt(pos_);
}

int CommandSequence::Iterator::next(bool isReleaseBegin)
{
	if (!isReleaseBegin && pos_ == -1) return -1;

	if (!started_) {
		started_ = true;
		return pos_;
	}

	int next = -1;
	if (isReleaseBegin) {
		loopStack_.clear();
		isRelease_ = true;
		switch (seq_->release_.type) {
		case ReleaseType::NoRelease:
			break;
		case ReleaseType::FixedRelease:
			next = seq_->release_.begin;
			break;
		case ReleaseType::AbsoluteRelease:
		{
			int crtr;
			if (pos_ == -1) {
				int prevIdx = seq_->release_.begin - 1;
				if (prevIdx < 0) {
					next = seq_->release_.begin;
					break;
				}
				else {
					crtr = seq_->seq_[static_cast<size_t>(prevIdx)].type;
				}
			}
			else {
				crtr = seq_->seq_[static_cast<size_t>(pos_)].type;
			}

			for (size_t i = static_cast<size_t>(seq_->release_.begin); i < seq_->seq_.size(); ++i) {
				if (seq_->seq_[i].type <= crtr) {
					next = static_cast<int>(i);
					break;
				}
			}
			break;
		}
		case ReleaseType::RelativeRelease:
		{
			if (pos_ == -1) {
				int prevIdx = seq_->release_.begin - 1;
				if (prevIdx >= 0) relReleaseRatio_ = seq_->seq_[static_cast<size_t>(prevIdx)].type / 15.0f;
			}
			else {
				relReleaseRatio_ = seq_->seq_[static_cast<size_t>(pos_)].type / 15.0f;
			}
			next = seq_->release_.begin;
			break;
		}
		}
	}
	else {
		next = pos_ + 1;
	}

	while (!loopStack_.empty()) {
		if (pos_ == loopStack_.back().end) {
			if (loopStack_.back().times < 0) {	// Infinity loop
				next = loopStack_.back().begin;
				break;
			}
			else {
				if (loopStack_.back().times) {
					next = loopStack_.back().begin;
					--loopStack_.back().times;
					break;
				}
				else {
					loopStack_.pop_back();
				}
			}
		}
		else {
			break;
		}
	}

	for (auto& l : seq_->loops_) {
		if (next < l.begin) break;
		else if (next == l.begin) {
			if (loopStack_.empty()) {
				loopStack_.push_back({ l.begin, l.end, (l.times == 1) ? -1 : (l.times - 1)});
			}
			else {
				bool flag = true;
				for (auto& lp : loopStack_) {
					if (lp.begin == l.begin && lp.end == l.end) {
						flag = false;
						break;
					}
				}
				if (flag) {
					loopStack_.push_back({ l.begin, l.end, (l.times == 1) ? -1 : (l.times - 1)});
				}
			}
		}
	}
	pos_ = next;

	if (!isRelease_ && pos_ == seq_->release_.begin) {
		pos_ = -1;
	}
	else if (pos_ == static_cast<int>(seq_->seq_.size())) {
		pos_ = -1;
	}

	return pos_;
}

int CommandSequence::Iterator::front()
{
	started_ = true;
	loopStack_.clear();
	isRelease_ = false;
	relReleaseRatio_ = 1;

	if (seq_->release_.begin == 0) {
		pos_ = -1;
	}
	else {
		pos_ = 0;

		for (auto& l : seq_->loops_) {
			if (pos_ < l.begin) break;
			else if (pos_ == l.begin) {
				loopStack_.push_back({ l.begin, l.end, (l.times == 1) ? -1 : (l.times - 1)});
			}
		}
	}

	return pos_;
}

int CommandSequence::Iterator::end()
{
	pos_ = -1;
	started_ = false;
	return -1;
}
