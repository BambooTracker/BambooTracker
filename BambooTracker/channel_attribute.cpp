#include "channel_attribute.hpp"

int ChannelAttribute::chCountFM_ = 6;
int ChannelAttribute::chCountPSG_ = 3;

ChannelAttribute::ChannelAttribute(int number)
{
	setNumber(number);
}

void ChannelAttribute::setNumber(int number)
{
	number_ = number;
	// UNDONE: change channel type by Effect mode
	if (number < chCountFM_) {
		source_ = SoundSource::FM;
		idInSource_ = number;
	}
	else {
		source_ = SoundSource::PSG;
		idInSource_ = number - chCountFM_;
	}
}

int ChannelAttribute::getNumber() const
{
	return number_;
}

SoundSource ChannelAttribute::getSoundSource() const
{
	return source_;
}

int ChannelAttribute::getIdInSoundSource() const
{
	return idInSource_;
}
