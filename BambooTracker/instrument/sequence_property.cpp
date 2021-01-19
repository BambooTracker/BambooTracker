/*
 * Copyright (C) 2018-2021 Rerrah
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

#include "sequence_property.hpp"
#include "utils.hpp"

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
	  type(InstrumentSequenceExtendUnit::UnusedSubdata),
	  subdata(InstrumentSequenceBaseUnit::ERR_DATA)
{
}

InstrumentSequenceExtendUnit::InstrumentSequenceExtendUnit(int d, SubdataType subType, int subData) noexcept
	: InstrumentSequenceBaseUnit(d), type(subType), subdata(subData)
{
}

InstrumentSequenceExtendUnit InstrumentSequenceExtendUnit::makeOnlyDataUnit(int data) noexcept
{
	return InstrumentSequenceExtendUnit(data, SubdataType::UnusedSubdata, ERR_DATA);
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

InstrumentSequenceExtendUnit InstrumentSequenceExtendUnit::makeUnitWithDecode(int data, int subsrc) noexcept
{
	SubdataType type;
	if (subsrc & 0x20000) type = SubdataType::ShiftSubdata;
	else type = (subsrc & 0x10000) ? SubdataType::RatioSubdata : SubdataType::RawSubdata;
	return InstrumentSequenceExtendUnit(data, type, subsrc);
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
		auto&& it = utils::findIf(childs_, [pos](const auto& pair) {
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
	for (auto& pair : childs_) {
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
	for (auto& pair : childs_) {
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

InstrumentSequenceRelease::InstrumentSequenceRelease(ReleaseType type, int beginPos)
	: type_(type),
	  begin_(type_ == ReleaseType::NoRelease ? DISABLE_RELEASE : beginPos)
{
}

void InstrumentSequenceRelease::setType(ReleaseType type)
{
	type_ = type;
	if (type == ReleaseType::NoRelease) begin_ = DISABLE_RELEASE;
}
void InstrumentSequenceRelease::setBeginPos(int pos)
{
	if (type_ != ReleaseType::NoRelease) begin_ = pos;
}

void InstrumentSequenceRelease::disable()
{
	type_ = ReleaseType::NoRelease;
	begin_ = DISABLE_RELEASE;
}
