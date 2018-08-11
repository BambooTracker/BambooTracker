#include "pattern.hpp"

Pattern::Pattern(int num)
	: num_(num), size_(64), steps_(64)
{
}

Step& Pattern::getStep(int num)
{
	return steps_.at(num);
}

size_t Pattern::getSize() const
{
	return size_;
}

void Pattern::changeSize(size_t size)
{
	size_ = size;
	if (steps_.size() < size)
		steps_.resize(size);
}
