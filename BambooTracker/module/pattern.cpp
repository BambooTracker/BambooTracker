#include "pattern.hpp"
#include "effect.hpp"
#include "misc.hpp"
#include <algorithm>

Pattern::Pattern(int n, size_t defSize)
	: num_(n), size_(defSize), steps_(defSize), usedCnt_(0)
{
}

Pattern::Pattern(int n, size_t size, std::vector<Step> steps)
	: num_(n), size_(size), steps_(steps), usedCnt_(0)
{
}

void Pattern::setNumber(int n)
{
	num_ = n;
}

int Pattern::getNumber() const
{
	return num_;
}

int Pattern::usedCountUp()
{
	return ++usedCnt_;
}

int Pattern::usedCountDown()
{
	return --usedCnt_;
}

int Pattern::getUsedCount() const
{
	return usedCnt_;
}

Step& Pattern::getStep(int n)
{
	return steps_.at(static_cast<size_t>(n));
}

size_t Pattern::getSize() const
{
	for (size_t i = 0; i < size_; ++i) {
		for (int j = 0; j < 4; ++j) {
			switch (Effect::makeEffectData(	// "SoundSource::FM" is dummy
						SoundSource::FM, steps_[i].getEffectID(j), steps_[i].getEffectValue(j)
						).type) {
			case EffectType::PositionJump:
			case EffectType::SongEnd:
			case EffectType::PatternBreak:
				return i + 1;
			default:
				break;
			}
		}
	}
	return size_;
}

void Pattern::changeSize(size_t size)
{
	if (0 < size && size <= 256) {
		size_ = size;
		if (steps_.size() < size) steps_.resize(size);
	}
}

void Pattern::insertStep(int n)
{
	if (n < static_cast<int>(size_))
		steps_.emplace(steps_.begin() + n);
}

void Pattern::deletePreviousStep(int n)
{
	if (!n) return;

	steps_.erase(steps_.begin() + n - 1);
	if (steps_.size() < size_)
		steps_.resize(size_);
}

bool Pattern::existCommand() const
{
	auto&& it = std::find_if(steps_.begin(), steps_.begin() + static_cast<int>(size_),
							 [](const Step& step) { return step.existCommand(); });
	return (it != steps_.end());
}

std::vector<int> Pattern::getEditedStepIndices() const
{
	std::vector<int> list;
	for (size_t i = 0; i < size_; ++i) {
		if (steps_.at(i).existCommand())
			list.push_back(static_cast<int>(i));
	}
	return list;
}

std::unordered_set<int> Pattern::getRegisteredInstruments() const
{
	std::unordered_set<int> set;
	for (size_t i = 0; i < size_; ++i) {
		int n = steps_.at(i).getInstrumentNumber();
		if (n > -1) set.insert(n);
	}
	return set;
}

Pattern Pattern::clone(int asNumber)
{
	return Pattern(asNumber, size_, steps_);
}

void Pattern::clear()
{
	steps_ = std::vector<Step>(size_);
}
