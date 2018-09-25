#include "pattern.hpp"

Pattern::Pattern(int n)
	: num_(n), size_(64), steps_(64)
{
}

int Pattern::getNumber() const
{
	return num_;
}

Step& Pattern::getStep(int n)
{
	return steps_.at(n);
}

size_t Pattern::getSize() const
{
	size_t ret = 0;
	for (size_t i = 0; i < size_; ++i) {
		auto id = steps_[i].getEffectID();
		if ((id == "0B" || id == "0C" || id == "0D") && steps_[i].getEffectValue() != -1)
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
