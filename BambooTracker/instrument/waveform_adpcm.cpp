#include "waveform_adpcm.hpp"

WaveformADPCM::WaveformADPCM(int num)
	: AbstractInstrumentProperty (num),
	  rootKeyNum_(DEF_RT_KEY_),
	  rootDeltaN_(DEF_RT_DELTAN_),
	  isRepeated_(DEF_REPET_),
	  samples_(1)
{
}

WaveformADPCM::WaveformADPCM(const WaveformADPCM& other)
	: AbstractInstrumentProperty(other)
{
	rootKeyNum_ = other.rootKeyNum_;
	rootDeltaN_ = other.rootDeltaN_;
	isRepeated_ = other.isRepeated_;
	samples_ = other.samples_;
}

bool operator==(const WaveformADPCM& a, const WaveformADPCM& b) {
	return (a.rootKeyNum_ == b.rootKeyNum_ && a.rootDeltaN_ == b.rootDeltaN_
			&& a.isRepeated_ == b.isRepeated_ && a.samples_ == b.samples_);
}

std::unique_ptr<WaveformADPCM> WaveformADPCM::clone()
{
	std::unique_ptr<WaveformADPCM> clone = std::make_unique<WaveformADPCM>(*this);
	clone->clearUserInstruments();
	return clone;
}

void WaveformADPCM::setRootKeyNumber(int n)
{
	rootKeyNum_ = n;
}

int WaveformADPCM::getRootKeyNumber() const
{
	return rootKeyNum_;
}

void WaveformADPCM::setRootDeltaN(int dn)
{
	rootDeltaN_ = dn;
}

int WaveformADPCM::getRootDeltaN() const
{
	return rootDeltaN_;
}

void WaveformADPCM::setRepeatEnabled(bool enabled)
{
	isRepeated_ = enabled;
}

bool WaveformADPCM::isRepeatable() const
{
	return isRepeated_;
}

void WaveformADPCM::storeSamples(std::vector<uint8_t> samples)
{
	samples_ = samples;
}

std::vector<uint8_t> WaveformADPCM::getSamples() const
{
	return samples_;
}

bool WaveformADPCM::isEdited() const
{
	if (rootKeyNum_ != DEF_RT_KEY_
			|| rootDeltaN_ != DEF_RT_DELTAN_
			|| isRepeated_ != DEF_REPET_
			|| samples_.size() != 1
			|| samples_[0] != 0)
		return true;
	return false;
}
