#pragma once

#include <memory>
#include <string>
#include "instruments_manager.hpp"
#include "binary_container.hpp"
#include "format/wopn_file.h"

class InstrumentIO
{
public:
	static void saveInstrument(BinaryContainer& ctr, std::weak_ptr<InstrumentsManager> instMan, int instNum);
	static AbstractInstrument* loadInstrument(BinaryContainer& ctr,
											  std::string path,
											  std::weak_ptr<InstrumentsManager> instMan,
											  int instNum);

private:
	static AbstractInstrument* loadBTIFile(BinaryContainer& ctr,
										   std::weak_ptr<InstrumentsManager> instMan,
										   int instNum);
	static AbstractInstrument* loadDMPFile(BinaryContainer& ctr,std::string path,
										   std::weak_ptr<InstrumentsManager> instMan,
										   int instNum);
	static AbstractInstrument* loadTFIFile(BinaryContainer& ctr,std::string path,
										   std::weak_ptr<InstrumentsManager> instMan,
										   int instNum);
	static AbstractInstrument* loadVGIFile(BinaryContainer& ctr,std::string path,
										   std::weak_ptr<InstrumentsManager> instMan,
										   int instNum);
	static AbstractInstrument* loadOPNIFile(BinaryContainer& ctr,
											std::weak_ptr<InstrumentsManager> instMan,
											int instNum);
	static AbstractInstrument* loadY12File(BinaryContainer& ctr,std::string path,
										   std::weak_ptr<InstrumentsManager> instMan,
										   int instNum);
	static AbstractInstrument* loadINSFile(BinaryContainer& ctr,
										   std::weak_ptr<InstrumentsManager> instMan,
										   int instNum);

public:
	static AbstractInstrument* loadWOPNInstrument(const WOPNInstrument &srcInst,
												  std::weak_ptr<InstrumentsManager> instMan,
												  int instNum);
	static AbstractInstrument* loadBTBInstrument(const BinaryContainer& instCtr,
												 const BinaryContainer& propCtr,
												 std::weak_ptr<InstrumentsManager> instMan,
												 int instNum,
												 uint32_t bankVersion);
	static AbstractInstrument* loadPPCInstrument(const std::vector<uint8_t> sample,
												 std::weak_ptr<InstrumentsManager> instMan,
												 int instNum);

private:
	static size_t loadInstrumentPropertyOperatorSequenceForInstrument(
			FMEnvelopeParameter param, size_t instMemCsr, std::weak_ptr<InstrumentsManager> instMan,
			BinaryContainer& ctr, InstrumentFM* inst, int idx, uint32_t version);
	static size_t getPropertyPositionForBTB(const BinaryContainer& propCtr, uint8_t subsecType, uint8_t index);
	static int convertTFIVGMDT(int dt);


private:
	InstrumentIO();
};
