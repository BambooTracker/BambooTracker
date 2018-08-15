#pragma once

#include "abstruct_command.hpp"
#include <memory>
#include "module.hpp"

class SetInstrumentToStepCommand : public AbstructCommand
{
public:
	SetInstrumentToStepCommand(std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int stepNum, int instNum);
	void redo() override;
	void undo() override;
	int getID() const override;
	bool mergeWith(const AbstructCommand* other) override;

	int getSong() const;
	int getTrack() const;
	int getOrder() const;
	int getStep() const;
	int getInst() const;

private:
	std::weak_ptr<Module> mod_;
	int song_, track_, order_, step_, inst_;
	int prevInst_;
	bool isComplete_;
};
