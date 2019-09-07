#pragma once

#include "abstract_command.hpp"
#include <memory>
#include <vector>
#include "module.hpp"

class ReplaceInstrumentInPatternCommand : public AbstractCommand
{
public:
	ReplaceInstrumentInPatternCommand(std::weak_ptr<Module> mod, int songNum,
									  int beginTrack, int beginOrder, int beginStep,
									  int endTrack, int endStep, int newInst);
	  void redo() override;
	  void undo() override;
	  CommandId getID() const override;

  private:
	  std::weak_ptr<Module> mod_;
	  int song_;
	  int bTrack_, order_, bStep_;
	  int eTrack_, eStep_;
	  int inst_;
	  std::vector<int> prevInsts_;
};
