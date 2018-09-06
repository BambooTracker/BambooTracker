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

	// Chip details
	int getGateCount(SoundSource src, int ch) const;

	// Stream samples
	void getStreamSamples(int16_t* container, size_t nSamples);

	// Stream details
	int getRate() const;
	int getDuration() const;

private:
	chip::OPNA opna_;

	void initChip();

	/*----- FM -----*/
public:
	// Key on-off
	void keyOnFM(int ch, Note note, int octave, int fine);
	void keyOffFM(int ch);
	void resetFMChannelEnvelope(int ch);

	// Set Instrument
	void setInstrumentFM(int ch, std::shared_ptr<InstrumentFM> inst);
	void updateInstrumentFM(int instNum);
	void updateInstrumentFMEnvelopeParameter(int envNum, FMEnvelopeParameter param);
	void setInstrumentFMOperatorEnabled(int envNum, int opNum);
	void updateInstrumentFMLFOParameter(int lfoNum, FMLFOParamter param);

	// Set volume
	void setVolumeFM(int ch, int volume);

	// Mute
	void setMuteFMState(int ch, bool isMuteFM);
	bool isMuteFM(int ch);

	// Chip details
	bool isKeyOnFM(int ch) const;
	bool enableFMEnvelopeReset(int ch) const;
	ToneDetail getFMTone(int ch) const;

private:
	std::shared_ptr<InstrumentFM> refInstFM_[6];
	std::unique_ptr<EnvelopeFM> envFM_[6];
	bool isKeyOnFM_[6];
	uint8_t fmOpEnables_[6];
	ToneDetail toneFM_[6];
	int volFM_[6];
	bool isMuteFM_[6];
	int gateCntFM_[6];
	bool enableEnvResetFM_[6];
	int lfoFreq_;

	/// bit0: right on/off
	/// bit1: left on/off
	uint8_t panFM_[6];

	uint32_t getFmChannelMask(int ch);
	uint32_t getFMChannelOffset(int ch);
	void writeFMEnvelopeToRegistersFromInstrument(int ch);
	void writeFMEnveropeParameterToRegister(int ch, FMEnvelopeParameter param, int value);
	void writeFMLFOAllRegisters(int ch);
	void writeFMLFORegister(int ch, FMLFOParamter param);
	void checkLFOUsed();
	void setInstrumentFMProperties(int ch);

	bool isCareer(int op, int al);

	inline uint8_t calculateTL(int ch, uint8_t data) const
	{
		return (data > 127 - volFM_[ch]) ? 127 : (data + volFM_[ch]);
	}

	/*----- SSG -----*/
public:
	// Key on-off
	void keyOnSSG(int ch, Note note, int octave, int fine);
	void keyOffSSG(int ch);

	// Set Instrument
	void setInstrumentSSG(int ch, std::shared_ptr<InstrumentSSG> inst);
	void updateInstrumentSSG(int instNum);

	// Set volume
	void setVolumeSSG(int ch, int volume);

	// Mute
	void setMuteSSGState(int ch, bool isMuteFM);
	bool isMuteSSG(int ch);

	// Chip details
	bool isKeyOnSSG(int ch) const;
	ToneDetail getSSGTone(int ch) const;

private:
	void setInstrumentSSGProperties(int ch);

	inline uint8_t judgeSSEGRegisterValue(int v) {
		return (v == -1) ? 0 : (0x08 + v);
	}

private:
	std::shared_ptr<InstrumentSSG> refInstSSG_[3];
	uint8_t mixerSSG_;
	ToneDetail toneSSG_[3];
	int volSSG_[3];
	bool isMuteSSG_[3];
	int gateCntSSG_[3];
};
