#pragma once

#include "abstruct_command.hpp"
#include <memory>
#include "module.hpp"

class SetVolumeToStepCommand : public AbstructCommand
{
public:
	SetVolumeToStepCommand(std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int stepNum, int volume);
	void redo() override;
	void undo() override;
	int getID() const override;
	bool mergeWith(const AbstructCommand* other) override;

	int getSong() const;
	int getTrack() const;
	int getOrder() const;
	int getStep() const;
	int getVol() const;

private:
	std::weak_ptr<Module> mod_;
	int song_, track_, order_, step_, vol_;
	int prevVol_;
	bool isComplete_;
};
