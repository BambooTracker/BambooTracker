/*
 * Copyright (C) 2018-2019 Rerrah
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <memory>
#include <string>
#include "../abstract_command.hpp"
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
