#pragma once

#include <string>
#include "misc.hpp"

class AbstructInstrument
{
public:
	int getNumber() const;
	SoundSource getSoundSource() const;
	std::string getName() const;
	void setName(std::string name);

protected:
	AbstructInstrument(int number, SoundSource source, std::string name);

private:
	int number_;
	SoundSource source_;
	std::string name_;
};
