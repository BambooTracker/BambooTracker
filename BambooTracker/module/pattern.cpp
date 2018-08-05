#include "pattern.hpp"

size_t Pattern::rowSize_ = 64;

Pattern::Pattern(int num) :
	num_(num)
{
	for (int i = 0; i < rowSize_; ++i) {
		rows_.push_back(std::make_unique<Step>());
	}
}

/********** Static **********/
void Pattern::changeRowSize(size_t size)
{
	rowSize_ = size;
}
