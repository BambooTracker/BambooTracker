#pragma once

#include <vector>
#include <unordered_set>
#include <cstddef>
#include "step.hpp"

class Pattern
{
public:
	Pattern(int n, size_t defSize);

	void setNumber(int n);
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
	std::unordered_set<int> getRegisteredInstruments() const;

	Pattern clone(int asNumber);

	void transpose(int seminotes, std::vector<int> excludeInsts);

	void clear();

private:
	int num_;
	size_t size_;
	std::vector<Step> steps_;
	int usedCnt_;

	Pattern(int n, size_t size, std::vector<Step> steps);
};
