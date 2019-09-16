#pragma once

#include "abstract_command.hpp"
#include <memory>
#include "module.hpp"

class SetPatternToOrderCommand : public AbstractCommand
{
public:
	SetPatternToOrderCommand(std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int patternNum, bool secondEntry);
	void redo() override;
	void undo() override;
	CommandId getID() const override;
	bool mergeWith(const AbstractCommand* other) override;

	int getSong() const;
	int getTrack() const;
	int getOrder() const;
	bool isSecondEntry() const;
	int getPattern() const;

private:
	std::weak_ptr<Module> mod_;
	int song_, track_, order_, pattern_;
	int prevPattern_;
	bool isSecond_;
};
