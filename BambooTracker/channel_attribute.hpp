#pragma once

#include "misc.hpp"

class ChannelAttribute
{
public:
	ChannelAttribute(int number);
	void setNumber(int number);
	int getNumber() const;
	SoundSource getSoundSource() const;
	int getIdInSoundSource() const;

	// UNDONE: change channel type by Effect mode

private:
	int number_;
	SoundSource source_;
	int idInSource_;

	static int chCountFM_, chCountPSG_;
};
