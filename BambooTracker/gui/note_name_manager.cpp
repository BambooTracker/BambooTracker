/*
 * Copyright (C) 2021 Rerrah
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

#include "note_name_manager.hpp"
#include <unordered_map>
#include "configuration.hpp"
#include "enum_hash.hpp"

namespace
{
const std::unordered_map<NoteNotationSystem, QStringList> NAMES = {
	{
		NoteNotationSystem::ENGLISH,
		{
			"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
		}
	},
	{
		NoteNotationSystem::GERMAN,
		{
			"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "B", "H"
		}
	}
};
}

std::unique_ptr<NoteNameManager> NoteNameManager::instance_;

NoteNameManager& NoteNameManager::getManager()
{
	if (instance_)
		return *instance_;

	NoteNameManager *out = new NoteNameManager;
	instance_.reset(out);
	return *out;
}

NoteNameManager::NoteNameManager() : list_(&NAMES.at(NoteNotationSystem::ENGLISH)) {}

NoteNameManager::~NoteNameManager() = default;

void NoteNameManager::setNotationSystem(NoteNotationSystem system)
{
	list_ = &NAMES.at(system);
}

QString NoteNameManager::getNoteString(int noteNum)
{
	return QString("%1%2").arg(list_->at(noteNum % 12), -2, QChar('-')).arg(noteNum / 12);
}
