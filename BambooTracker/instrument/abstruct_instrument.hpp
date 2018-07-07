#pragma once

#include <string>
#include <memory>
#include "misc.hpp"

class AbstructInstrument
{
public:
	virtual ~AbstructInstrument() = default;

	int getNumber() const;
	SoundSource getSoundSource() const;
	std::string getName() const;
	void setName(std::string name);
	virtual std::unique_ptr<AbstructInstrument> clone() = 0;

protected:
	AbstructInstrument(int number, SoundSource source, std::string name);

private:
	int number_;
	SoundSource source_;
	std::string name_;	// UTF-8
};
