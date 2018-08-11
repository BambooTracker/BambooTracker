#pragma once

#include <vector>
#include <cstddef>
#include "step.hpp"

class Pattern
{
public:
	explicit Pattern(int num);

	Step& getStep(int num);

	size_t getSize() const;
	void changeSize(size_t size);

private:
	int num_;
	size_t size_;
	std::vector<Step> steps_;
};
