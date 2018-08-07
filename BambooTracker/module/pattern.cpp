#include "pattern.hpp"

size_t Pattern::rowSize_ = 64;

Pattern::Pattern(int num)
	: num_(num), rows_(rowSize_)
{
}

/********** Static **********/
void Pattern::changeRowSize(size_t size)
{
	rowSize_ = size;
}
