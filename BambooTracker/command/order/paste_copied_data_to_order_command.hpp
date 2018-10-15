#pragma once

#include "abstract_command.hpp"
#include <memory>
#include <vector>
#include <string>
#include "module.hpp"

class PasteCopiedDataToOrderCommand : public AbstractCommand
{
public:
	PasteCopiedDataToOrderCommand(std::weak_ptr<Module> mod, int songNum, int beginTrack, int beginOrder,
								  std::vector<std::vector<std::string>> cells);
  void redo() override;
  void undo() override;
  int getID() const override;

private:
  std::weak_ptr<Module> mod_;
  int song_, track_, order_;
  std::vector<std::vector<std::string>> cells_, prevCells_;

  void setCells(std::vector<std::vector<std::string>>& cells);
};
