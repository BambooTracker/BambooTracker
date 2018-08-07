#pragma once

#include <cstdint>
#include <memory>
#include "opna.hpp"
#include "instrument.hpp"
#include "instruments_manager.hpp"
#include "misc.hpp"

class OPNAController
{
public:
	#ifdef SINC_INTERPOLATION
	OPNAController(int clock, int rate, int duration, InstrumentsManager* im);
	#else
	OPNAController(int clock, int rate, InstrumentsManager* im);
	#endif

	// Reset and initialize
	void reset();

	// Key on-off
	void keyOnFM(int ch, Note note, int octave, int fine);
	void keyOnPSG(int ch, Note note, int octave, int fine);
	void keyOffFM(int ch);
	void keyOffPSG(int ch);

	// Set Instrument
	void setInstrumentFM(int ch, std::shared_ptr<InstrumentFM> inst);
	void setInstrumentPSG(int ch, std::shared_ptr<InstrumentPSG> inst);
	void updateInstrumentFM(int instNum);
	void setInstrumentFMEnvelopeParameter(int envNum, FMParameter param);
	void setInstrumentFMOperatorEnable(int envNum, int opNum);

	// Set volume
	void setVolumePSG(int ch, int level);

	// Stream samples
	void getStreamSamples(int16_t* container, size_t nSamples);

	// Stream details
	int getRate() const;
	int getDuration() const;

private:
	chip::OPNA opna_;
	std::shared_ptr<InstrumentFM> instFM_[6];
	std::shared_ptr<InstrumentPSG> instPSG_[3];
	bool isKeyOnFM_[6];
	uint8_t fmOpEnables_[6];
	uint8_t mixerPSG_;

	void initChip();
	uint32_t getFmChannelMask(int ch);
	uint32_t getFMChannelOffset(int ch);
	void updateFMEnvelopeRegisters(int ch);
};
