#include "pattern.hpp"

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
	return steps_.at(n);
}

size_t Pattern::getSize() const
{
	for (size_t i = 0; i < size_; ++i) {
		if (steps_[i].checkEffectID("0B") != -1
				|| steps_[i].checkEffectID("0C") != -1
				|| steps_[i].checkEffectID("0D") != -1)
			return i + 1;
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
	for (size_t i = 0; i < size_; ++i) {
		if (steps_.at(i).existCommand())
			return true;
	}
	return false;
}

std::vector<int> Pattern::getEditedStepIndices() const
{
	std::vector<int> list;
	for (size_t i = 0; i < size_; ++i) {
		if (steps_.at(i).existCommand())
			list.push_back(i);
	}
	return list;
}

std::set<int> Pattern::getRegisteredInstruments() const
{
	std::set<int> set;
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
