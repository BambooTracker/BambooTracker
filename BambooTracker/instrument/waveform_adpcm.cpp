#include "waveform_adpcm.hpp"

#include "chips/codec/ymb_codec.hpp"	// TODO: remove
WaveformADPCM::WaveformADPCM(int num)
	: AbstractInstrumentProperty (num),
	  rootKeyNum_(DEF_RT_KEY_),
	  rootDeltaN_(DEF_RT_DELTAN_),
	  isRepeated_(DEF_REPET_),
	  samples_(1),
	  startAddress_(0),
	  stopAddress_(0)
{
	// TODO: remove  dummy
	const int rate = 16000;
	rootDeltaN_ = static_cast<int>(std::round((rate << 16) / 55500.));
	rootKeyNum_ = 60;	//c5
	int len = 1;
	int l = len * rate;
	std::vector<int16_t> w(l);
	for (int i = 0; i < l; ++i) {
		w[i] = static_cast<int16_t>(32000 * std::sin(2. * 3.14159265359 * 261.626 * i / rate));
	}
	samples_ = std::vector<uint8_t>((w.size() + 1) / 2);
	ymb_encode(w.data(), samples_.data(), w.size());
	// ---------------------
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
			/*|| samples_.size() != 1	// TODO: uncommented
			|| samples_[0] != 0*/)
		return true;
	return false;
}
