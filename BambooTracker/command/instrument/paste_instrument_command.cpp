#include "paste_instrument_command.hpp"
#include "misc.hpp"

PasteInstrumentCommand::PasteInstrumentCommand(InstrumentsManager &manager, int num, int refNum) :
	manager_(manager)
{
	oldInst_ = manager.getInstrumentSharedPtr(num)->clone();
	refInst_ = manager.getInstrumentSharedPtr(refNum)->clone();
}

void PasteInstrumentCommand::redo()
{
	manager_.setInstrumentName(oldInst_->getNumber(), refInst_->getName());
	switch (refInst_->getSoundSource()) {
	case SoundSource::FM:
		manager_.setInstrumentFMEnvelope(oldInst_->getNumber(),
										 dynamic_cast<InstrumentFM*>(refInst_.get())->getEnvelopeNumber());
		break;
	case SoundSource::PSG:
		// UNDONE
		break;
	}
}

void PasteInstrumentCommand::undo()
{
	manager_.setInstrumentName(oldInst_->getNumber(), oldInst_->getName());
	switch (oldInst_->getSoundSource()) {
	case SoundSource::FM:
		manager_.setInstrumentFMEnvelope(oldInst_->getNumber(),
										 dynamic_cast<InstrumentFM*>(oldInst_.get())->getEnvelopeNumber());
		break;
	case SoundSource::PSG:
		// UNDONE
		break;
	}
}

int PasteInstrumentCommand::getID() const
{
	return 3;
}
