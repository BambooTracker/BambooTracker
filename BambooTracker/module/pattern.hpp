#pragma once

#include <vector>
#include <cstddef>
#include "step.hpp"

class Pattern
{
public:
	explicit Pattern(int num);

	static void changeRowSize(size_t size);

private:
	int num_;
	std::vector<Step> rows_;

	static size_t rowSize_;
};
