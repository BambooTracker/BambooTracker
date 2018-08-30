#pragma once

#include <cstdint>
#include <memory>
#include "opna.hpp"
#include "instrument.hpp"
#include "misc.hpp"

struct ToneDetail
{
	int octave;
	Note note;
	int fine;
};

class OPNAController
{
public:
	#ifdef SINC_INTERPOLATION
	OPNAController(int clock, int rate, int duration);
	#else
	OPNAController(int clock, int rate);
	#endif

	// Reset and initialize
	void reset();

	// Key on-off
	void keyOnFM(int ch, Note note, int octave, int fine);
	void keyOnSSG(int ch, Note note, int octave, int fine);
	void keyOffFM(int ch);
	void keyOffSSG(int ch);
	void resetChannelEnvelope(int ch);

	// Set Instrument
	void setInstrumentFM(int ch, std::shared_ptr<InstrumentFM> inst);
	void setInstrumentSSG(int ch, std::shared_ptr<InstrumentSSG> inst);
	void updateInstrumentFM(int instNum);
	void updateInstrumentFMEnvelopeParameter(int envNum, FMParameter param);
	void setInstrumentFMOperatorEnable(int envNum, int opNum);

	// Set volume
	void setVolumeFM(int ch, int volume);
	void setVolumeSSG(int ch, int volume);

	// Mute
	void setMuteFMState(int ch, bool isMuteFM);
	void setMuteSSGState(int ch, bool isMuteFM);
	bool isMuteFM(int ch);
	bool isMuteSSG(int ch);

	// Chip details
	bool isKeyOnFM(int ch) const;
	bool isKeyOnSSG(int ch) const;
	bool enableEnvelopeReset(int ch) const;
	ToneDetail getFMTone(int ch) const;
	ToneDetail getSSGTone(int ch) const;

	// Stream samples
	void getStreamSamples(int16_t* container, size_t nSamples);

	// Stream details
	int getRate() const;
	int getDuration() const;

private:
	chip::OPNA opna_;
	std::shared_ptr<InstrumentFM> refInstFM_[6];
	std::shared_ptr<InstrumentSSG> refInstSSG_[3];
	std::unique_ptr<EnvelopeFM> envFM_[6];
	bool isKeyOnFM_[6];
	uint8_t fmOpEnables_[6];
	uint8_t mixerSSG_;
	ToneDetail toneFM_[6], toneSSG_[3];
	int volFM_[6], volSSG_[3];
	bool isMuteFM_[6], isMuteSSG_[3];
	bool enableEnvResetFM_[6];

	void initChip();
	uint32_t getFmChannelMask(int ch);
	uint32_t getFMChannelOffset(int ch);
	void writeFMEnvelopeToRegistersFromInstrument(int ch);
	void writeFMEnveropeParameterToRegister(int ch, FMParameter param, int value);
	void setInstrumentFMProperties(int ch);

	bool isCareer(int op, int al);

	inline uint8_t judgeSSEGRegisterValue(int v) {
		return (v == -1) ? 0 : (0x08 + v);
	}

	inline uint8_t calculateTL(int ch, uint8_t data) const
	{
		return (data > 127 - volFM_[ch]) ? 127 : (data + volFM_[ch]);
	}
};
