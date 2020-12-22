/*
 * Copyright (C) 2018-2020 Rerrah
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

#include "bank.hpp"
#include <cstdio>
#include "io/instrument_io.hpp"
#include "io/opni_io.hpp"
#include "io/btb_io.hpp"
#include "io/ff_io.hpp"
#include "io/ppc_io.hpp"
#include "io/pvi_io.hpp"
#include "io/dat_io.hpp"
#include "format/wopn_file.h"

BtBank::BtBank(const std::vector<int>& ids, const std::vector<std::string>& names)
	: ids_(ids),
	  names_(names)
{
}

BtBank::BtBank(const std::vector<int>& ids, const std::vector<std::string>& names,
			   const std::vector<io::BinaryContainer>& instSecs, const io::BinaryContainer& propSec, uint32_t version)
	: instCtrs_(instSecs),
	  propCtr_(propSec),
	  ids_(ids),
	  names_(names),
	  version_(version)
{
}

size_t BtBank::getNumInstruments() const
{
	return ids_.size();
}

std::string BtBank::getInstrumentIdentifier(size_t index) const
{
	return std::to_string(ids_.at(index));
}

std::string BtBank::getInstrumentName(size_t index) const
{
	return names_.at(index);
}

AbstractInstrument* BtBank::loadInstrument(size_t index, std::weak_ptr<InstrumentsManager> instMan, int instNum) const
{
	return io::BtbIO::loadInstrument(instCtrs_.at(static_cast<size_t>(index)), propCtr_, instMan, instNum, version_);
}

/******************************/
void WopnBank::WOPNDeleter::operator()(WOPNFile *x)
{
	WOPN_Free(x);
}

struct WopnBank::InstEntry
{
	WOPNInstrument *inst;
	struct ValuesType
	{
		bool percussive : 1;
		unsigned msb : 7;
		unsigned lsb : 7;
		unsigned nth : 7;
	} vals;
};

WopnBank::WopnBank(WOPNFile* wopn)
	: wopn_(wopn)
{
	unsigned numM = wopn->banks_count_melodic;
	unsigned numP = wopn->banks_count_percussion;

	size_t instMax = 128 * (numP + numM);
	entries_.reserve(instMax);

	for (size_t i = 0; i < instMax; ++i) {
		InstEntry ent;
		ent.vals.percussive = (i / 128) >= numM;
		WOPNBank& bank = ent.vals.percussive
						 ? wopn->banks_percussive[(i / 128) - numM]
						 : wopn->banks_melodic[i / 128];
		ent.vals.msb = bank.bank_midi_msb;
		ent.vals.lsb = bank.bank_midi_lsb;
		ent.vals.nth = i % 128;
		ent.inst = &bank.ins[ent.vals.nth];
		if ((ent.inst->inst_flags & WOPN_Ins_IsBlank) == 0)
			entries_.push_back(ent);
	}

	entries_.shrink_to_fit();
}

size_t WopnBank::getNumInstruments() const
{
	return entries_.size();
}

std::string WopnBank::getInstrumentIdentifier(size_t index) const
{
	const InstEntry& ent = entries_.at(index);
	char identifier[64];
	std::sprintf(identifier, "%c%03d:%03d:%03d", "MP"[ent.vals.percussive],
			ent.vals.msb, ent.vals.lsb, ent.vals.nth);
	return identifier;
}

std::string WopnBank::getInstrumentName(size_t index) const
{
	const InstEntry& ent = entries_.at(index);
	return ent.inst->inst_name;
}

AbstractInstrument* WopnBank::loadInstrument(size_t index, std::weak_ptr<InstrumentsManager> instMan, int instNum) const
{
	const InstEntry& ent = entries_.at(index);
	return io::OpniIO::loadWOPNInstrument(*ent.inst, instMan, instNum);
}

/******************************/
FfBank::FfBank(const std::vector<int>& ids, const std::vector<std::string>& names, const std::vector<io::BinaryContainer>& ctrs)
	: ids_(ids), names_(names), instCtrs_(ctrs)
{
}

size_t FfBank::getNumInstruments() const
{
	return ids_.size();
}

std::string FfBank::getInstrumentIdentifier(size_t index) const
{
	return std::to_string(ids_.at(index));
}

std::string FfBank::getInstrumentName(size_t index) const
{
	return names_.at(index);
}

AbstractInstrument* FfBank::loadInstrument(size_t index, std::weak_ptr<InstrumentsManager> instMan, int instNum) const
{
	return io::FfIO::loadInstrument(instCtrs_.at(index), names_.at(index), instMan, instNum);
}

void FfBank::setInstrumentName(size_t index, const std::string& name)
{
	names_.at(index) = name;
}

/******************************/
PpcBank::PpcBank(const std::vector<int>& ids, const std::vector<std::vector<uint8_t>>& samples)
	: ids_(ids), samples_(samples)
{
}

size_t PpcBank::getNumInstruments() const
{
	return samples_.size();
}

std::string PpcBank::getInstrumentIdentifier(size_t index) const
{
	return std::to_string(ids_.at(index));
}

std::string PpcBank::getInstrumentName(size_t index) const
{
	(void)index;
	return "";
}

AbstractInstrument* PpcBank::loadInstrument(size_t index, std::weak_ptr<InstrumentsManager> instMan, int instNum) const
{
	return io::PpcIO::loadInstrument(samples_.at(index), instMan, instNum);
}

/******************************/
PviBank::PviBank(const std::vector<int>& ids, const std::vector<std::vector<uint8_t>>& samples)
	: ids_(ids), samples_(samples)
{
}

size_t PviBank::getNumInstruments() const
{
	return samples_.size();
}

std::string PviBank::getInstrumentIdentifier(size_t index) const
{
	return std::to_string(ids_.at(index));
}

std::string PviBank::getInstrumentName(size_t index) const
{
	(void)index;
	return "";
}

AbstractInstrument* PviBank::loadInstrument(size_t index, std::weak_ptr<InstrumentsManager> instMan, int instNum) const
{
	return io::PviIO::loadInstrument(samples_.at(index), instMan, instNum);
}

/******************************/
Mucom88Bank::Mucom88Bank(const std::vector<int>& ids, const std::vector<std::string>& names, const std::vector<io::BinaryContainer>& ctrs)
	: ids_(ids), names_(names), instCtrs_(ctrs)
{
}

size_t Mucom88Bank::getNumInstruments() const
{
	return ids_.size();
}

std::string Mucom88Bank::getInstrumentIdentifier(size_t index) const
{
	return std::to_string(ids_.at(index));
}

std::string Mucom88Bank::getInstrumentName(size_t index) const
{
	return names_.at(index);
}

AbstractInstrument* Mucom88Bank::loadInstrument(size_t index, std::weak_ptr<InstrumentsManager> instMan, int instNum) const
{
	return io::DatIO::loadInstrument(instCtrs_.at(index), names_.at(index), instMan, instNum);
}

void Mucom88Bank::setInstrumentName(size_t index, const std::string& name)
{
	names_.at(index) = name;
}
