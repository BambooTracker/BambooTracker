#pragma once

#include "abstract_command.hpp"
#include <memory>
#include "module.hpp"
#include "misc.hpp"

class SetEffectValueToStepCommand : public AbstractCommand
{
public:
	SetEffectValueToStepCommand(std::weak_ptr<Module> mod, int songNum, int trackNum,
								int orderNum, int stepNum, int n, int value,
								EffectDisplayControl ctrl, bool secondEntry);
	void redo() override;
	void undo() override;
	CommandId getID() const override;
	bool mergeWith(const AbstractCommand* other) override;

	int getSong() const;
	int getTrack() const;
	int getOrder() const;
	int getStep() const;
	int getN() const;
	bool isSecondEntry() const;
	int getEffectValue() const;

private:
	std::weak_ptr<Module> mod_;
	int song_, track_, order_, step_, n_;
	int val_, prevVal_;
	EffectDisplayControl ctrl_;
	bool isSecond_;
};
