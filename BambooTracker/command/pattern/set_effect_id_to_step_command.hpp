#pragma once

#include "abstruct_command.hpp"
#include <memory>
#include <string>
#include "module.hpp"

class SetEffectIDToStepCommand : public AbstructCommand
{
public:
	SetEffectIDToStepCommand(std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int stepNum, int n, std::string id);
	void redo() override;
	void undo() override;
	int getID() const override;
	bool mergeWith(const AbstructCommand* other) override;

	int getSong() const;
	int getTrack() const;
	int getOrder() const;
	int getStep() const;
	int getN() const;
	std::string getEffectID() const;

private:
	std::weak_ptr<Module> mod_;
	int song_, track_, order_, step_, n_;
	std::string effID_, prevEffID_;
	bool isComplete_;
};
