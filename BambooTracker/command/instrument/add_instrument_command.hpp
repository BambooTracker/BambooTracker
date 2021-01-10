/*
 * Copyright (C) 2018-2020 Rerrah
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
#include "instrument.hpp"
#include "instruments_manager.hpp"

class AddInstrumentCommand : public AbstractCommand
{
public:
	AddInstrumentCommand(std::weak_ptr<InstrumentsManager> manager,
						 int num, InstrumentType type, const std::string& name);
	AddInstrumentCommand(std::weak_ptr<InstrumentsManager> manager,
						 std::unique_ptr<AbstractInstrument> inst);
	void redo() override;
	void undo() override;

private:
	std::weak_ptr<InstrumentsManager> manager_;
	int num_;
	InstrumentType type_;
	std::string name_;
	std::unique_ptr<AbstractInstrument> inst_;
};
