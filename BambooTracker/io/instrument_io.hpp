/*
 * Copyright (C) 2019-2020 Rerrah
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

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
