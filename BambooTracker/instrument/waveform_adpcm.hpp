#pragma once

#include <vector>
#include <cmath>
#include <cstdint>
#include "abstract_instrument_property.hpp"

class SampleADPCM : public AbstractInstrumentProperty
{
public:
	explicit SampleADPCM(int num);
	SampleADPCM(const SampleADPCM& other);

	friend bool operator==(const SampleADPCM& a, const SampleADPCM& b);
	friend bool operator!=(const SampleADPCM& a, const SampleADPCM& b) { return !(a == b); }

	std::unique_ptr<SampleADPCM> clone();

	void setRootKeyNumber(int n);
	int getRootKeyNumber() const;
	void setRootDeltaN(int dn);
	int getRootDeltaN() const;
	void setRepeatEnabled(bool enabled);
	bool isRepeatable() const;
	void storeSample(std::vector<uint8_t> sample);
	std::vector<uint8_t> getSamples() const;
	void clearSample();
	void setStartAddress(size_t addr);
	size_t getStartAddress() const;
	void setStopAddress(size_t addr);
	size_t getStopAddress() const;

	bool isEdited() const override;
	void clearParameters() override;

private:
	int rootKeyNum_, rootDeltaN_;
	bool isRepeated_;
	std::vector<uint8_t> sample_;
	size_t startAddress_, stopAddress_;

	static constexpr int DEF_RT_KEY_ = 60;	// C5
	static constexpr int DEF_RT_DELTAN_ = 0x49cd;	// 16000Hz
	static constexpr bool DEF_REPET_ = false;
};
