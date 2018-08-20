#pragma once

#include <vector>
#include <cstddef>
#include "step.hpp"

class Pattern
{
public:
	explicit Pattern(int n);

	int getNumber() const;

	Step& getStep(int n);

	size_t getSize() const;
	void changeSize(size_t size);

	void insertStep(int n);
	void deletePreviousStep(int n);

private:
	int num_;
	size_t size_;
	std::vector<Step> steps_;
};
