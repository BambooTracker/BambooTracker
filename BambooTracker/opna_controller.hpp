#pragma once

#include <cstdint>
#include <memory>
#include "opna.hpp"
#include "instrument.hpp"
#include "misc.hpp"

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
	void keyOnFM(int id, Note note, int octave, int fine);
	void keyOnPSG(int id, Note note, int octave, int fine);
	void keyOffFM(int id);
	void keyOffPSG(int id);

	// Set Instrument
	void setInstrumentFM(int id, InstrumentFM* inst);
	void setInstrumentPSG(int id, InstrumentPSG* inst);
	void setInstrumentFMParameter(int instNum, FMParameter param, int value);
	void setInstrumentFMOperatorEnable(int instNum, int opNum, bool enable);

	// Set volume
	void setVolumePSG(int id, int level);

	// Stream samples
	void getStreamSamples(int16_t* container, size_t nSamples);

	// Stream details
	int getRate() const;
	int getDuration() const;

private:
	chip::OPNA opna_;
	std::unique_ptr<InstrumentFM> instFM_[6];
	std::unique_ptr<InstrumentPSG> instPSG_[3];
	bool isKeyOnFM_[6];
	uint8_t fmOpEnables_[6];
	uint8_t mixerPSG_;

	void initChip();
	uint32_t getFmChannelMask(int id);
	uint32_t getFMChannelOffset(int id);
};
