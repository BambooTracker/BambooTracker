#include "bank.hpp"
#include "instrument_io.hpp"
#include "format/wopn_file.h"
#include <stdio.h>
#include <utility>

void WopnBank::WOPNDeleter::operator()(WOPNFile *x) {
	WOPN_Free(x);
}

struct WopnBank::InstEntry {
	WOPNInstrument *inst;
	struct {
		bool percussive : 1;
		unsigned msb : 7;
		unsigned lsb : 7;
		unsigned nth : 7;
	};
};

WopnBank::WopnBank(WOPNFile *wopn) : wopn_(wopn) {
	unsigned numM = wopn->banks_count_melodic;
	unsigned numP = wopn->banks_count_percussion;

	size_t instMax = 128 * (numP + numM);
	entries_.reserve(instMax);

	for (size_t i = 0; i < instMax; ++i) {
		InstEntry ent;
		ent.percussive = (i / 128) >= numM;
		WOPNBank &bank = ent.percussive ?
			wopn->banks_percussive[(i / 128) - numM] :
			wopn->banks_melodic[i / 128];
		ent.msb = bank.bank_midi_msb;
		ent.lsb = bank.bank_midi_lsb;
		ent.nth = i % 128;
		ent.inst = &bank.ins[ent.nth];
		if ((ent.inst->inst_flags & WOPN_Ins_IsBlank) == 0)
			entries_.push_back(ent);
	}

	entries_.shrink_to_fit();
}

WopnBank::~WopnBank() {
}

size_t WopnBank::getNumInstruments() const {
	return entries_.size();
}

std::string WopnBank::getInstrumentIdentifier(size_t index) const {
	const InstEntry &ent = entries_.at(index);
	char identifier[64];
	sprintf(identifier, "%c%03d:%03d:%03d", "MP"[ent.percussive], ent.msb, ent.lsb, ent.nth);
	return identifier;
}

std::string WopnBank::getInstrumentName(size_t index) const {
	const InstEntry &ent = entries_.at(index);
	return ent.inst->inst_name;
}

AbstractInstrument* WopnBank::loadInstrument(size_t index, std::weak_ptr<InstrumentsManager> instMan, int instNum) const {
	const InstEntry &ent = entries_.at(index);
	return InstrumentIO::loadWOPNInstrument(*ent.inst, instMan, instNum);
}

BtBank::BtBank(std::vector<int> ids, std::vector<std::string> names)
	: ids_(std::move(ids)),
	  names_(std::move(names))
{
}

BtBank::BtBank(BinaryContainer instSec, BinaryContainer propSec)
{
	// TODO: implement
}

BtBank::~BtBank()
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
	// TODO: implement
}
