#pragma once

#include "abstract_command.hpp"
#include <memory>
#include <string>
#include "module.hpp"

class SetEffectIDToStepCommand : public AbstractCommand
{
public:
	SetEffectIDToStepCommand(std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int stepNum,
							 int n, std::string id, bool fillValue00, bool secondEntry);
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
	std::string getEffectID() const;

private:
	std::weak_ptr<Module> mod_;
	int song_, track_, order_, step_, n_;
	std::string effID_, prevEffID_;
	bool filledValue00_;
	bool isSecond_;
};
