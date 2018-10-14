#pragma once

#include <vector>

class Groove
{
public:
	Groove();
	Groove(std::vector<int> seq);
	void setSequrnce(std::vector<int> seq);
	std::vector<int> getSequence() const;

private:
	std::vector<int> seq_;
};
