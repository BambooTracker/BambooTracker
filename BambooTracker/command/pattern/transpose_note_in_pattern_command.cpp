/*
 * Copyright (C) 2020 Rerrah
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

#include "transpose_note_in_pattern_command.hpp"
#include "misc.hpp"

TransposeNoteInPatternCommand::TransposeNoteInPatternCommand(std::weak_ptr<Module> mod,
															 int songNum, int beginTrack,
															 int beginOrder, int beginStep,
															 int endTrack, int endStep, int seminote)
	: mod_(mod),
	  song_(songNum),
	  bTrack_(beginTrack),
	  order_(beginOrder),
	  bStep_(beginStep),
	  eTrack_(endTrack),
	  eStep_(endStep),
	  seminote_(seminote)
{
	auto& sng = mod.lock()->getSong(songNum);

	for (int step = beginStep; step <= endStep; ++step) {
		for (int track = beginTrack; track <= endTrack; ++track) {
			int n = sng.getTrack(track).getPatternFromOrderNumber(beginOrder)
					.getStep(step).getNoteNumber();
			if (n > -1) prevKeys_.push_back(n);
		}
	}
}

void TransposeNoteInPatternCommand::redo()
{
	auto& sng = mod_.lock()->getSong(song_);

	for (int step = bStep_; step <= eStep_; ++step) {
		for (int track = bTrack_; track <= eTrack_; ++track) {
			auto& s = sng.getTrack(track).getPatternFromOrderNumber(order_).getStep(step);
			int n = s.getNoteNumber();
			if (n > -1) {
				s.setNoteNumber(clamp(n + seminote_, 0, 95));
			}
		}
	}
}

void TransposeNoteInPatternCommand::undo()
{
	auto& sng = mod_.lock()->getSong(song_);

	size_t i = 0;
	for (int step = bStep_; step <= eStep_; ++step) {
		for (int track = bTrack_; track <= eTrack_; ++track) {
			auto& s = sng.getTrack(track).getPatternFromOrderNumber(order_).getStep(step);
			if (s.getNoteNumber() > -1) s.setNoteNumber(prevKeys_.at(i++));
		}
	}
}

CommandId TransposeNoteInPatternCommand::getID() const
{
	return CommandId::TransposeNoteInPattern;
}
