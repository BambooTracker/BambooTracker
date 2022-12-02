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

#pragma once
#include <vector>
#include <memory>
#include <map>
#include <deque>
#include <algorithm>
#include <utility>
#include "abstract_instrument_property.hpp"
#include "sequence_iterator_interface.hpp"

struct InstrumentSequenceBaseUnit
{
	int data;

	static constexpr int ERR_DATA = -1;

	InstrumentSequenceBaseUnit() noexcept;
	explicit InstrumentSequenceBaseUnit(int d) noexcept;
	virtual ~InstrumentSequenceBaseUnit() = default;

	friend bool operator==(const InstrumentSequenceBaseUnit& a, const InstrumentSequenceBaseUnit& b)
	{
		return a.data == b.data;
	}

	friend bool operator!=(const InstrumentSequenceBaseUnit& a, const InstrumentSequenceBaseUnit& b) { return !(a == b); }
};

struct InstrumentSequenceExtendUnit : public InstrumentSequenceBaseUnit
{
	enum SubdataType : int
	{
		UnusedSubdata,
		RawSubdata,
		RatioSubdata,
		ShiftSubdata
	} type;
	///		- If bit 17 is 0,
	///			* If bit 16 is 0, bit 0-15 is raw data
	///			* If bit 16 is 1, bit 0-7 is 2nd and bit 8-15 is 1st ratio value
	///		- If bit 17 is 1,
	///			* If bit 16 is 0, bit 0-15 is right shift value
	///			* If bit 16 is 1, bit 0-15 is left shift value
	int subdata;

	InstrumentSequenceExtendUnit() noexcept;
	explicit InstrumentSequenceExtendUnit(int d, SubdataType subType, int subData) noexcept;

	static InstrumentSequenceExtendUnit makeOnlyDataUnit(int data) noexcept;
	static InstrumentSequenceExtendUnit makeRawUnit(int data, int sub) noexcept;
	static InstrumentSequenceExtendUnit makeRatioUnit(int data, int subFirst, int subSecond) noexcept;
	static InstrumentSequenceExtendUnit makeShiftUnit(int data, int rshift) noexcept;
	static InstrumentSequenceExtendUnit makeUnitWithDecode(int data, int subsrc) noexcept;

	void getSubdataAsRaw(int& raw) const noexcept;
	void getSubdataAsRatio(int& first, int& second) const noexcept;
	void getSubdataAsShift(int& rshift) const noexcept;

	friend bool operator==(const InstrumentSequenceExtendUnit& a, const InstrumentSequenceExtendUnit& b)
	{
		return a.data == b.data && a.type == b.type && a.subdata == b.subdata;
	}

	friend bool operator!=(const InstrumentSequenceExtendUnit& a, const InstrumentSequenceExtendUnit& b) { return !(a == b); }
};

class InstrumentSequenceLoop
{
public:
	using Ptr = std::shared_ptr<InstrumentSequenceLoop>;

	static constexpr int INFINITE_LOOP = 1;
	/// Loop in a closed interval [begin, end]
	InstrumentSequenceLoop(int begin, int end, int times = INFINITE_LOOP);

	friend bool operator==(const InstrumentSequenceLoop& a, const InstrumentSequenceLoop& b)
	{
		return (a.begin_ == b.begin_ && a.end_ == b.end_ && a.times_ == b.times_);
	}

	friend bool operator!=(const InstrumentSequenceLoop& a, const InstrumentSequenceLoop& b) { return !(a == b); }

	void setBeginPos(int pos);
	inline int getBeginPos() const noexcept { return begin_; }
	void setEndPos(int pos);
	inline int getEndPos() const noexcept { return end_; }
	inline void setTimes(int times) noexcept { times_ = times; }
	inline int getTimes() const noexcept { return times_; }
	inline bool isInfinite() const noexcept { return times_ == INFINITE_LOOP; }

	inline Ptr getInnerLoopBeginAt(int pos) const { return childs_.at(pos); }

	bool addInnerLoop(const InstrumentSequenceLoop& inner);
	bool changeInnerLoop(int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop);
	void removeInnerLoop(int begin, int end);
	void removeAllInnerLoops();
	std::vector<InstrumentSequenceLoop> getAllInnerLoops()const;

	inline bool hasInnerLoop() const { return !childs_.empty(); }
	inline bool hasInnerLoopBeginAt(int pos) const { return childs_.count(pos); }
	bool isOverlapped(int begin, int end) const;
	inline bool isOverlapped(const InstrumentSequenceLoop& other) const
	{
		return isOverlapped(other.begin_, other.end_);
	}
	bool isContainable(int begin, int end) const;
	inline bool isContainable(const InstrumentSequenceLoop& other) const
	{
		return isContainable(other.begin_, other.end_);
	}
	bool hasSameRegion(int begin, int end) const;
	inline bool hasSameRegion(const InstrumentSequenceLoop& other) const
	{
		return hasSameRegion(other.begin_, other.end_);
	}
	InstrumentSequenceLoop clone() const;

protected:
	int begin_, end_, times_;
	std::map<int, Ptr> childs_;
};

class InstrumentSequenceLoopRoot : public InstrumentSequenceLoop
{
public:
	InstrumentSequenceLoopRoot(int size)
		: InstrumentSequenceLoop(0, size - 1) {}

	inline int size() const { return end_; }
	inline void extend() { setEndPos(end_ + 1); }
	inline void shrink() { if (end_) setEndPos(end_ - 1); }
	inline void resize(int size) { setEndPos(size - 1); }
	inline void clear()
	{
		removeAllInnerLoops();
		setEndPos(0);
	}
	inline Ptr getLoopBeginAt(int pos) const
	{
		return getInnerLoopBeginAt(pos);
	}
	inline bool addLoop(const InstrumentSequenceLoop& loop)
	{
		return addInnerLoop(loop);
	}
	inline bool changeLoop(int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop)
	{
		return changeInnerLoop(prevBegin, prevEnd, loop);
	}
	inline void removeLoop(int begin, int end)
	{
		removeInnerLoop(begin, end);
	}
	inline void removeAllLoops()
	{
		removeAllInnerLoops();
	}
	inline std::vector<InstrumentSequenceLoop> getAllLoops() const
	{
		return getAllInnerLoops();
	}
	inline bool hasLoop() const
	{
		return hasInnerLoop();
	}
	inline bool hasLoopBeginAt(int pos) const
	{
		return hasInnerLoopBeginAt(pos);
	}
	InstrumentSequenceLoopRoot clone() const;
};

namespace inst_utils
{
class LoopStack
{
private:
	struct StackItem
	{
		InstrumentSequenceLoop::Ptr loop;
		int count;
		bool isInfinite;

		explicit StackItem(const InstrumentSequenceLoop::Ptr& ptr);
	};
	std::deque<StackItem> stack_;

public:
	explicit LoopStack(const std::shared_ptr<InstrumentSequenceLoopRoot>& ptr);
	void clear();
	void pushLoopsAtPos(int pos);
	int checkLoopEndAndNextPos(int curPos);
};
}

class InstrumentSequenceRelease
{
public:
	enum ReleaseType
	{
		NoRelease,
		FixedRelease,
		AbsoluteRelease,
		RelativeRelease
	};

	static constexpr int DISABLE_RELEASE = -1;

	explicit InstrumentSequenceRelease(ReleaseType getType, int getBeginPos = DISABLE_RELEASE);

	friend bool operator==(const InstrumentSequenceRelease& a, const InstrumentSequenceRelease& b)
	{
		return (a.type_ == b.type_ && a.begin_ == b.begin_);
	}

	friend bool operator!=(const InstrumentSequenceRelease& a, const InstrumentSequenceRelease& b) { return !(a == b); }

	inline ReleaseType getType() const noexcept { return type_; }
	void setType(ReleaseType type);
	void setBeginPos(int pos);
	inline int getBeginPos() const noexcept { return begin_; }

	inline bool isEnabled() const noexcept
	{
		return (type_ != ReleaseType::NoRelease && begin_ != DISABLE_RELEASE);
	}

	void disable();

private:
	ReleaseType type_;
	int begin_;
};

template<class T>
class InstrumentSequenceProperty final : public AbstractInstrumentProperty
{
public:
	InstrumentSequenceProperty(int num, SequenceType defaultType, const T& defaultUnit, const T& errorUnit, int relReleaseDenom = 1)
		: AbstractInstrumentProperty(num),
		  DEF_TYPE_(defaultType),
		  DEF_UNIT_(defaultUnit),
		  ERR_UNIT_(errorUnit),
		  REL_RELEASE_DENOM_(relReleaseDenom),
		  loop_(std::make_shared<InstrumentSequenceLoopRoot>(1)),
		  release_(InstrumentSequenceRelease::NoRelease)
	{
		clearParameters();
	}

	friend bool operator==(const InstrumentSequenceProperty& a, const InstrumentSequenceProperty& b)
	{
		return (a.type_ == b.type_ && a.seq_ == b.seq_ && a.loop_ == b.loop_ && a.release_ == b.release_);
	}

	friend bool operator!=(const InstrumentSequenceProperty& a, const InstrumentSequenceProperty& b) { return !(a == b); }

	std::unique_ptr<InstrumentSequenceProperty> clone()
	{
		std::unique_ptr<InstrumentSequenceProperty> clone = std::make_unique<InstrumentSequenceProperty>(*this);
		clone->clearUserInstruments();
		return clone;
	}

	inline void setType(SequenceType type) noexcept { type_ = type; }
	inline SequenceType getType() const noexcept { return type_; }

	bool isEdited() const override
	{
		return (seq_.size() > 1 || seq_.front() != DEF_UNIT_ || loop_->hasLoop() || release_.getType() != InstrumentSequenceRelease::NoRelease);
	}

	void clearParameters() override
	{
		type_ = DEF_TYPE_;
		seq_ = { DEF_UNIT_ };
		loop_->clear();
		release_.disable();
	}

	//***** Sequence *****
	inline size_t getSequenceSize() const { return seq_.size(); }
	T getSequenceUnit(int n) const { return seq_.at(static_cast<size_t>(n)); }
	inline std::vector<T> getSequence() const { return seq_; }

	void addSequenceUnit(const T& unit) {
		seq_.push_back(unit);
		loop_->extend();
	}

	void removeSequenceUnit()
	{
		seq_.pop_back();
		loop_->shrink();
		if (release_.getBeginPos() == static_cast<int>(seq_.size()))
			release_.disable();
	}

	void setSequenceUnit(int n, const T& unit) { seq_.at(static_cast<size_t>(n)) = unit; }

	//***** Loop *****
	inline InstrumentSequenceLoopRoot getLoopRoot() const { return *loop_; }
	inline void addLoop(const InstrumentSequenceLoop& loop) const { loop_->addLoop(loop); }
	inline void removeLoop(int begin, int end) const { loop_->removeLoop(begin, end); }
	inline void clearLoops() const { loop_->removeAllLoops(); }
	inline void changeLoop(int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop)
	{
		loop_->changeLoop(prevBegin, prevEnd, loop);
	}

	//***** Release *****
	InstrumentSequenceRelease getRelease() const noexcept { return release_; }
	inline void setRelease(const InstrumentSequenceRelease& release) { release_ = release; }

	class Iterator final : public SequenceIteratorInterface<T>
	{
	public:
		explicit Iterator(const InstrumentSequenceProperty* seqProp)
			: SequenceIteratorInterface<T>(0),
			  seqProp_(seqProp),
			  loopStack_(seqProp->loop_),
			  isRelease_(false),
			  relReleaseRate_(1.)
		{
		}

		SequenceType type() const override
		{
			return seqProp_->type_;
		}

		T data() const override
		{
			if (this->hasEnded() || static_cast<int>(seqProp_->seq_.size()) <= this->pos_)
				return seqProp_->ERR_UNIT_;
			return (isRelease_ ? seqProp_->getSequenceUnit(this->pos_, relReleaseRate_)
							   : seqProp_->getSequenceUnit(this->pos_));
		}

		int next() override
		{
			if (this->hasEnded()) return this->END_SEQ_POS;

			if (this->state_ == SequenceIteratorState::NotBegin) {
				return this->pos_;
			}

			this->pos_ = loopStack_.checkLoopEndAndNextPos(this->pos_);
			loopStack_.pushLoopsAtPos(this->pos_);

			// Range check
			if ((!isRelease_ && seqProp_->release_.isEnabled() && this->pos_ >= seqProp_->release_.getBeginPos())
					|| this->pos_ >= static_cast<int>(seqProp_->seq_.size())) {
				this->pos_ = this->END_SEQ_POS;
			}

			return this->pos_;
		}

		int front() override
		{
			this->state_ = SequenceIteratorState::Run;
			loopStack_.clear();
			isRelease_ = false;
			relReleaseRate_ = 1.;

			if (seqProp_->release_.getBeginPos()) {
				this->pos_ = 0;
				loopStack_.pushLoopsAtPos(0);
			}
			else {
				this->pos_ = this->END_SEQ_POS;
			}

			return this->pos_;
		}

		int release() override
		{
			const std::vector<T>& seq = seqProp_->seq_;
			const InstrumentSequenceRelease& release = seqProp_->release_;

			int next = this->END_SEQ_POS;
			isRelease_ = true;
			loopStack_.clear();
			switch (release.getType()) {
			case InstrumentSequenceRelease::NoRelease:
				break;
			case InstrumentSequenceRelease::FixedRelease:
			{
				next = release.getBeginPos();
				break;
			}
			case InstrumentSequenceRelease::AbsoluteRelease:
			{
				int crtr;
				if (this->pos_ == this->END_SEQ_POS) {
					if (int relBegin = release.getBeginPos()) {
						crtr = seq[static_cast<size_t>(relBegin - 1)].data;
					}
					else {
						next = relBegin;
						break;
					}
				}
				else {
					crtr = seq[static_cast<size_t>(this->pos_)].data;
				}

				auto&& it = std::find_if(seq.begin() + release.getBeginPos(), seq.end(),
										 [crtr](const T& unit) { return (unit.data <= crtr); });
				if (it != seq.end()) next = std::distance(seq.begin(), it);
				break;
			}
			case InstrumentSequenceRelease::RelativeRelease:
			{
				next = release.getBeginPos();
				if (this->hasEnded()) {
					if (next) relReleaseRate_ = seq[static_cast<size_t>(next - 1)].data / seqProp_->REL_RELEASE_DENOM_;
				}
				else {
					relReleaseRate_ = seq[static_cast<size_t>(this->pos_)].data / seqProp_->REL_RELEASE_DENOM_;
				}
				break;
			}
			}
			this->pos_ = next;

			if (next == this->END_SEQ_POS) {
				this->state_ = SequenceIteratorState::End;
			}
			else {
				this->state_ = SequenceIteratorState::RunRelease;
				loopStack_.pushLoopsAtPos(this->pos_);
			}

			return this->pos_;
		}

		int end() override
		{
			this->pos_ = this->END_SEQ_POS;
			this->state_ = SequenceIteratorState::End;
			return this->END_SEQ_POS;
		}

	private:
		const InstrumentSequenceProperty* seqProp_;
		inst_utils::LoopStack loopStack_;
		bool isRelease_;
		double relReleaseRate_;
	};

	std::unique_ptr<InstrumentSequenceProperty::Iterator> getIterator()
	{
		return std::make_unique<Iterator>(this);
	}

private:
	const SequenceType DEF_TYPE_;
	const T DEF_UNIT_;
	const T ERR_UNIT_;
	const double REL_RELEASE_DENOM_;

	SequenceType type_;
	std::vector<T> seq_;
	std::shared_ptr<InstrumentSequenceLoopRoot> loop_;
	InstrumentSequenceRelease release_;

	inline T getSequenceUnit(int n, double relRate) const
	{
		T unit = seq_.at(static_cast<size_t>(n));
		unit.data = static_cast<int>(unit.data * relRate);
		return unit;
	}
};
