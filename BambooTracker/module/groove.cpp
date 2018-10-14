#include "groove.hpp"
#include <utility>

Groove::Groove()
	: seq_{ 6, 6 }
{
}

Groove::Groove(std::vector<int> seq)
	: seq_(std::move(seq))
{
}

void Groove::setSequrnce(std::vector<int> seq)
{
	seq_ = std::move(seq);
}

std::vector<int> Groove::getSequence() const
{
	return seq_;
}
