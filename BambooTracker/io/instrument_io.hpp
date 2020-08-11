#pragma once

#include <memory>
#include <string>
#include "instruments_manager.hpp"
#include "binary_container.hpp"
#include "format/wopn_file.h"

class InstrumentIO
{
public:
	static void saveInstrument(BinaryContainer& ctr,
							   const std::weak_ptr<InstrumentsManager> instMan, int instNum);
	static AbstractInstrument* loadInstrument(const BinaryContainer& ctr,
											  const std::string& path,
											  std::weak_ptr<InstrumentsManager> instMan,
											  int instNum);

private:
	static AbstractInstrument* loadBTIFile(const BinaryContainer& ctr,
										   std::weak_ptr<InstrumentsManager> instMan,
										   int instNum);
	static AbstractInstrument* loadDMPFile(const BinaryContainer& ctr,std::string path,
										   std::weak_ptr<InstrumentsManager> instMan,
										   int instNum);
	static AbstractInstrument* loadTFIFile(const BinaryContainer& ctr,std::string path,
										   std::weak_ptr<InstrumentsManager> instMan,
										   int instNum);
	static AbstractInstrument* loadVGIFile(const BinaryContainer& ctr,std::string path,
										   std::weak_ptr<InstrumentsManager> instMan,
										   int instNum);
	static AbstractInstrument* loadOPNIFile(const BinaryContainer& ctr,
											std::weak_ptr<InstrumentsManager> instMan,
											int instNum);
	static AbstractInstrument* loadY12File(const BinaryContainer& ctr,std::string path,
										   std::weak_ptr<InstrumentsManager> instMan,
										   int instNum);
	static AbstractInstrument* loadINSFile(const BinaryContainer& ctr,
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
	static AbstractInstrument* loadFFInstrument(const BinaryContainer& instCtr,
												const std::string& name,
												std::weak_ptr<InstrumentsManager> instMan,
												int instNum);
	static AbstractInstrument* loadPPCInstrument(const std::vector<uint8_t> sample,
												 std::weak_ptr<InstrumentsManager> instMan,
												 int instNum);
	static AbstractInstrument* loadPVIInstrument(const std::vector<uint8_t> sample,
												 std::weak_ptr<InstrumentsManager> instMan,
												 int instNum);
	static AbstractInstrument* loadMUCOM88Instrument(const BinaryContainer& instCtr,
													 const std::string& name,
													 std::weak_ptr<InstrumentsManager> instMan,
													 int instNum);

private:
	static size_t loadInstrumentPropertyOperatorSequenceForInstrument(
			FMEnvelopeParameter param, size_t instMemCsr,
			std::shared_ptr<InstrumentsManager>& instManLocked,
			const BinaryContainer& ctr, InstrumentFM* inst, int idx, uint32_t version);
	static size_t getPropertyPositionForBTB(const BinaryContainer& propCtr,
											uint8_t subsecType, uint8_t index);
	static int convertDTInTFIVGIDMP(int dt);


private:
	InstrumentIO();
};
