#include "pattern.hpp"

Pattern::Pattern(int n)
	: num_(n), size_(64), steps_(64), usedCnt_(0)
{
}

Pattern::Pattern(int n, size_t size, std::vector<Step> steps)
	: num_(n), size_(size), steps_(steps), usedCnt_(0)
{
}

int Pattern::setNumber(int n)
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
	size_ = size;
	if (steps_.size() < size)
		steps_.resize(size);
}

void Pattern::insertStep(int n)
{
	if (n < size_)
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

Pattern Pattern::clone(int asNumber)
{
	return Pattern(asNumber, size_, steps_);
}

void Pattern::clear()
{
	steps_ = std::vector<Step>(size_);
}
