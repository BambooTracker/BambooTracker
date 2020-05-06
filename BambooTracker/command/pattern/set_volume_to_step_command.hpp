#pragma once

#include "abstract_command.hpp"
#include <memory>
#include "module.hpp"

class SetVolumeToStepCommand : public AbstractCommand
{
public:
	SetVolumeToStepCommand(std::weak_ptr<Module> mod, int songNum, int trackNum,
						   int orderNum, int stepNum, int volume, bool isFMReversed, bool secondEntry);
	void redo() override;
	void undo() override;
	CommandId getID() const override;
	bool mergeWith(const AbstractCommand* other) override;

	int getSong() const;
	int getTrack() const;
	int getOrder() const;
	int getStep() const;
	bool isSecondEntry() const;
	int getVol() const;

private:
	std::weak_ptr<Module> mod_;
	int song_, track_, order_, step_, vol_;
	int prevVol_;
	bool isFMReserved_;
	bool isSecond_;
};
