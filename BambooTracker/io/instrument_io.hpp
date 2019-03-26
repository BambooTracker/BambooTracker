#pragma once

#include <memory>
#include <string>
#include "instruments_manager.hpp"
#include "binary_container.hpp"
#include "format/wopn_file.h"

class InstrumentIO
{
public:
	static void saveInstrument(std::string path, std::weak_ptr<InstrumentsManager> instMan, int instNum);
	static AbstractInstrument* loadInstrument(std::string path,
											  std::weak_ptr<InstrumentsManager> instMan,
											  int instNum);

private:
	static AbstractInstrument* loadBTIFile(std::string path,
										   std::weak_ptr<InstrumentsManager> instMan,
										   int instNum);
	static AbstractInstrument* loadDMPFile(std::string path,
										   std::weak_ptr<InstrumentsManager> instMan,
										   int instNum);
	static AbstractInstrument* loadTFIFile(std::string path,
										   std::weak_ptr<InstrumentsManager> instMan,
										   int instNum);
	static AbstractInstrument* loadVGIFile(std::string path,
										   std::weak_ptr<InstrumentsManager> instMan,
										   int instNum);
	static AbstractInstrument* loadOPNIFile(std::string path,
										   std::weak_ptr<InstrumentsManager> instMan,
										   int instNum);
	static AbstractInstrument* loadY12File(std::string path,
										   std::weak_ptr<InstrumentsManager> instMan,
										   int instNum);
	static AbstractInstrument* loadINSFile(std::string path,
										   std::weak_ptr<InstrumentsManager> instMan,
										   int instNum);

public:
	static AbstractInstrument* loadWOPNInstrument(const WOPNInstrument &srcInst,
										   std::weak_ptr<InstrumentsManager> instMan,
										   int instNum);

private:
	static size_t loadInstrumentPropertyOperatorSequenceForInstrument(
			FMEnvelopeParameter param, size_t instMemCsr, std::weak_ptr<InstrumentsManager> instMan,
			BinaryContainer& ctr, InstrumentFM* inst, int idx, uint32_t version);
	static int convertTFIVGMDT(int dt);


private:
	InstrumentIO();
};
