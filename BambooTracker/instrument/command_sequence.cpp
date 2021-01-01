/*
 * Copyright (C) 2018-2020 Rerrah
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "command_sequence.hpp"

CommandSequence::CommandSequence(int num, SequenceType seqType, int comType, int comData)
	: AbstractInstrumentProperty(num),
	  DEF_COM_TYPE_(comType),
	  DEF_COM_DATA_(comData),
	  DEF_SEQ_TYPE_(seqType)
{
	clearParameters();
}

CommandSequence::CommandSequence(const CommandSequence& other)
	: AbstractInstrumentProperty(other),
	  DEF_COM_TYPE_(other.DEF_COM_TYPE_),
	  DEF_COM_DATA_(other.DEF_COM_DATA_),
	  DEF_SEQ_TYPE_(other.DEF_SEQ_TYPE_),
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
	return std::make_unique<Iterator>(this);
}

bool CommandSequence::isEdited() const
{
	return  (seq_.size() != 1 || seq_.front().type != DEF_COM_TYPE_ || seq_.front().data != DEF_COM_DATA_
																							|| loops_.size() || release_.begin > -1);
}

void CommandSequence::clearParameters()
{
	type_ = DEF_SEQ_TYPE_;
	seq_ = {{ DEF_COM_TYPE_, DEF_COM_DATA_ }};
	loops_.clear();
	release_ = { ReleaseType::NoRelease, -1 };
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
	return (pos_ == -1 || pos_ >= static_cast<long int>(seq_->getSequenceSize()))
			? -1
			: isRelease_
			  ? static_cast<int>(seq_->getSequenceTypeAt(pos_) * relReleaseRatio_)
			  : seq_->getSequenceTypeAt(pos_);
}

int CommandSequence::Iterator::getCommandData() const
{
	return (pos_ == -1
			|| pos_ >= static_cast<long int>(seq_->getSequenceSize())) ? -1
																	   : seq_->getSequenceDataAt(pos_);
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


//==================================================
InstrumentSequenceBaseUnit::InstrumentSequenceBaseUnit() noexcept
	: data(ERR_DATA)
{
}

InstrumentSequenceBaseUnit::InstrumentSequenceBaseUnit(int d) noexcept
	: data(d)
{
}

InstrumentSequenceExtendUnit::InstrumentSequenceExtendUnit() noexcept
	: InstrumentSequenceBaseUnit(),
	  type(InstrumentSequenceExtendUnit::RawSubdata),
	  subdata(InstrumentSequenceBaseUnit::ERR_DATA)
{
}

InstrumentSequenceExtendUnit::InstrumentSequenceExtendUnit(int d, SubdataType subType, int subData) noexcept
	: InstrumentSequenceBaseUnit(d), type(subType), subdata(subData)
{
}

InstrumentSequenceExtendUnit InstrumentSequenceExtendUnit::makeRawUnit(int data, int sub) noexcept
{
	return InstrumentSequenceExtendUnit(data, SubdataType::RawSubdata, sub);
}

InstrumentSequenceExtendUnit InstrumentSequenceExtendUnit::makeRatioUnit(int data, int subFirst, int subSecond) noexcept
{
	return InstrumentSequenceExtendUnit(data, SubdataType::RatioSubdata, ((1 << 16) | (subFirst << 8) | subSecond));
}

InstrumentSequenceExtendUnit InstrumentSequenceExtendUnit::makeShiftUnit(int data, int rshift) noexcept
{
	int sub = (rshift > 0) ? ((2 << 16) | rshift) : ((3 << 16) | -rshift);
	return InstrumentSequenceExtendUnit(data, SubdataType::ShiftSubdata, sub);
}

void InstrumentSequenceExtendUnit::getSubdataAsRaw(int& raw) const noexcept
{
	raw = subdata;
}

void InstrumentSequenceExtendUnit::getSubdataAsRatio(int& first, int& second) const noexcept
{
	first = (subdata & 0x0ff00) >> 8;
	second = subdata & 0x000ff;
}

void InstrumentSequenceExtendUnit::getSubdataAsShift(int& rshift) const noexcept
{
	rshift = (subdata & 0x10000) ? -(0x0ffff & data) : (0x0ffff & data);
}

namespace
{
inline InstrumentSequenceLoop::Ptr makeSequenceLoopPtr(const InstrumentSequenceLoop& loop)
{
	return std::make_shared<InstrumentSequenceLoop>(loop);
}
}

InstrumentSequenceLoop::InstrumentSequenceLoop(int begin, int end, int times)
	: begin_(begin), end_(end), times_(times)
{
}

void InstrumentSequenceLoop::setBeginPos(int pos)
{
	if (begin_ < pos && hasInnerLoop()) {
		auto&& it = std::find_if(childs_.begin(), childs_.end(), [pos](const auto& pair) {
			return pos <= pair.second->end_;
		});
		if (it == childs_.end()) {
			removeAllInnerLoops();
		}
		else {
			if (it->second->begin_ < pos) it->second->setBeginPos(pos);
			childs_.erase(childs_.begin(), it);
		}
	}
	begin_ = pos;
}

void InstrumentSequenceLoop::setEndPos(int pos)
{
	if (pos < end_ && hasInnerLoop()) {
		auto&& it = std::find_if(childs_.rbegin(), childs_.rend(), [pos](const auto& pair) {
			return pair.second->begin_ <= pos;
		});
		if (it == childs_.rend()) {
			removeAllInnerLoops();
		}
		else {
			if (pos < it->second->end_) it->second->setEndPos(pos);
			childs_.erase(it.base(), childs_.end());
		}
	}
	end_ = pos;
}

bool InstrumentSequenceLoop::addInnerLoop(const InstrumentSequenceLoop& inner)
{
	for (auto pair : childs_) {
		Ptr& loop = pair.second;
		if (loop->isOverlapped(inner)) {
			if (loop->hasSameRegion(inner)) {
				loop->times_ = inner.times_;
				return true;
			}
			else if (loop->isContainable(inner)) {
				return loop->addInnerLoop(inner);
			}
			else {	// Illegal region
				return false;
			}
		}
		else if (loop->begin_ < inner.begin_) break;
	}

	// Add new region
	childs_.insert(std::make_pair(inner.begin_, makeSequenceLoopPtr(inner)));
	return true;
}

bool InstrumentSequenceLoop::changeInnerLoop(int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop)
{
	if (hasInnerLoopBeginAt(prevBegin)) {
		InstrumentSequenceLoop::Ptr lpt = getInnerLoopBeginAt(prevBegin);
		if (lpt->end_ != prevEnd) return lpt->changeInnerLoop(prevBegin, prevEnd, loop);
		if (prevBegin != loop.begin_) lpt->setBeginPos(loop.begin_);
		if (prevEnd != loop.end_) lpt->setEndPos(loop.end_);
		if (lpt->times_ != loop.times_) lpt->setTimes(loop.times_);
		return true;
	}
	else {
		return false;
	}
}

void InstrumentSequenceLoop::removeInnerLoop(int begin, int end)
{
	for (auto pair : childs_) {
		Ptr& loop = pair.second;
		if (loop->isOverlapped(begin, end)) {
			if (loop->hasSameRegion(begin, end)) {
				childs_.erase(pair.first);
			}
			else if (loop->isContainable(begin, end)) {
				loop->removeInnerLoop(begin, end);
			}
			return;
		}
		else if (loop->begin_ < begin) return;
	}
}

void InstrumentSequenceLoop::removeAllInnerLoops()
{
	childs_.clear();
}

std::vector<InstrumentSequenceLoop> InstrumentSequenceLoop::getAllInnerLoops() const
{
	std::vector<InstrumentSequenceLoop> list;
	for (const auto& pair : childs_) {
		list.push_back(*pair.second);
		auto in = pair.second->getAllInnerLoops();
		list.insert(list.end(), in.begin(), in.end());
	}
	return list;
}

bool InstrumentSequenceLoop::isOverlapped(int begin, int end) const
{
	if (begin_ == begin) return true;
	if (begin_ < begin) return (begin <= end_);
	else /* begin < begin_ */ return (begin_ <= end);
}

bool InstrumentSequenceLoop::isContainable(int begin, int end) const
{
	return ((begin_ < begin && end <= end_)
			|| (begin_ == begin && end < end_));
}

bool InstrumentSequenceLoop::hasSameRegion(int begin, int end) const
{
	return (begin_ == begin && end_ == end);
}

InstrumentSequenceLoop InstrumentSequenceLoop::clone() const
{
	InstrumentSequenceLoop l(begin_, end_, times_);
	for (const auto& pair : childs_) {
		l.childs_.insert(std::make_pair(pair.first, makeSequenceLoopPtr(pair.second->clone())));
	}
	return l;
}

InstrumentSequenceLoopRoot InstrumentSequenceLoopRoot::clone() const
{
	InstrumentSequenceLoopRoot r(end_);
	for (const auto& pair : childs_) {
		r.childs_.insert(std::make_pair(pair.first, makeSequenceLoopPtr(pair.second->clone())));
	}
	return r;
}

namespace inst_utils
{
LoopStack::StackItem::StackItem(const InstrumentSequenceLoop::Ptr& ptr)
	: loop(ptr), count(ptr->getTimes()), isInfinite(ptr->isInfinite())
{
}

LoopStack::LoopStack(const std::shared_ptr<InstrumentSequenceLoopRoot>& ptr)
	: stack_{ StackItem(std::static_pointer_cast<InstrumentSequenceLoop>(ptr)) }
{
}

void LoopStack::clear()
{
	stack_.erase(stack_.begin() + 1, stack_.end());
}

void LoopStack::pushLoopsAtPos(int pos)
{
	while (true) {
		InstrumentSequenceLoop::Ptr& loop = stack_.back().loop;
		if (!loop->hasInnerLoopBeginAt(pos)) break;
		stack_.emplace_back(loop->getInnerLoopBeginAt(pos));
	}
}

int LoopStack::checkLoopEndAndNextPos(int curPos)
{
	while (stack_.size() > 1) {
		StackItem& item = stack_.back();
		if (item.loop->getEndPos() != curPos) {
			return curPos + 1;
		}
		if (item.isInfinite) {
			return item.loop->getBeginPos();
		}
		if (--(item.count)) {
			return item.loop->getBeginPos();
		}
		else {
			stack_.pop_back();
		}
	}
	return curPos + 1;
}
}

InstrumentSequenceRelease::InstrumentSequenceRelease(ReleaseTypeImproved type, int beginPos)
	: type_(type),
	  begin_(type_ == ReleaseTypeImproved::NoRelease ? DISABLE_RELEASE : beginPos)
{
}

void InstrumentSequenceRelease::setType(ReleaseTypeImproved type)
{
	type_ = type;
	if (type == ReleaseTypeImproved::NoRelease) begin_ = DISABLE_RELEASE;
}
void InstrumentSequenceRelease::setBeginPos(int pos)
{
	if (type_ != ReleaseTypeImproved::NoRelease) begin_ = pos;
}

void InstrumentSequenceRelease::disable()
{
	type_ = ReleaseTypeImproved::NoRelease;
	begin_ = DISABLE_RELEASE;
}
