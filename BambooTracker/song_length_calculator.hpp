#pragma once

#include "module.hpp"

class SongLengthCalculator
{
public:
	SongLengthCalculator(Module& mod, int songNum);
	double calculateBySecond() const;

private:
	Module& mod_;
	int songNum_;

	inline double getStrictStepTicks(int rate, int tempo, int speed) const
	{
		return 2.5 * rate * speed / tempo;
	}
};
