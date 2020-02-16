#include "waveform_adpcm.hpp"

WaveformADPCM::WaveformADPCM(int num)
	: AbstractInstrumentProperty (num),
	  rootKeyNum_(DEF_RT_KEY_),
	  rootDeltaN_(DEF_RT_DELTAN_),
	  isRepeated_(DEF_REPET_),
	  sample_(1),
	  startAddress_(0),
	  stopAddress_(0)
{
}

WaveformADPCM::WaveformADPCM(const WaveformADPCM& other)
	: AbstractInstrumentProperty(other)
{
	rootKeyNum_ = other.rootKeyNum_;
	rootDeltaN_ = other.rootDeltaN_;
	isRepeated_ = other.isRepeated_;
	sample_ = other.sample_;
}

bool operator==(const WaveformADPCM& a, const WaveformADPCM& b) {
	return (a.rootKeyNum_ == b.rootKeyNum_ && a.rootDeltaN_ == b.rootDeltaN_
			&& a.isRepeated_ == b.isRepeated_ && a.sample_ == b.sample_);
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

void WaveformADPCM::storeSample(std::vector<uint8_t> sample)
{
	sample_ = sample;
}

std::vector<uint8_t> WaveformADPCM::getSamples() const
{
	return sample_;
}

void WaveformADPCM::clearSample()
{
	sample_.clear();
}

void WaveformADPCM::setStartAddress(size_t addr)
{
	startAddress_ = addr;
}

size_t WaveformADPCM::getStartAddress() const
{
	return startAddress_;
}

void WaveformADPCM::setStopAddress(size_t addr)
{
	stopAddress_ = addr;
}

size_t WaveformADPCM::getStopAddress() const
{
	return stopAddress_;
}

bool WaveformADPCM::isEdited() const
{
	if (rootKeyNum_ != DEF_RT_KEY_
			|| rootDeltaN_ != DEF_RT_DELTAN_
			|| isRepeated_ != DEF_REPET_
			|| sample_.size() != 1
			|| sample_[0] != 0)
		return true;
	return false;
}
