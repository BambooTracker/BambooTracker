#pragma once

#include "abstract_command.hpp"
#include <memory>
#include "module.hpp"

class SetEffectValueToStepCommand : public AbstractCommand
{
public:
	SetEffectValueToStepCommand(std::weak_ptr<Module> mod, int songNum, int trackNum,
								int orderNum, int stepNum, int n, int value, bool isFMReversed);
	void redo() override;
	void undo() override;
	int getID() const override;
	bool mergeWith(const AbstractCommand* other) override;

	int getSong() const;
	int getTrack() const;
	int getOrder() const;
	int getStep() const;
	int getN() const;
	int getEffectValue() const;

private:
	std::weak_ptr<Module> mod_;
	int song_, track_, order_, step_, n_;
	int val_, prevVal_;
	bool isComplete_;
	bool isFMReserved_;
};
