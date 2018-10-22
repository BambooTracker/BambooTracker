#pragma once

#include <vector>
#include <cstddef>
#include "step.hpp"

class Pattern
{
public:
	explicit Pattern(int n);

	int setNumber(int n);
	int getNumber() const;

	int usedCountUp();
	int usedCountDown();
	int getUsedCount() const;

	Step& getStep(int n);

	size_t getSize() const;
	void changeSize(size_t size);

	void insertStep(int n);
	void deletePreviousStep(int n);

	bool existCommand() const;
	std::vector<int> getEditedStepIndices() const;

	Pattern clone(int asNumber);

	void clear();

private:
	int num_;
	size_t size_;
	std::vector<Step> steps_;
	int usedCnt_;

	Pattern(int n, size_t size, std::vector<Step> steps);
};
