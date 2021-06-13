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
const std::unordered_map<KeySignature::Type, QStringList> NAMES_EN = {
	{
		KeySignature::C,
		{
			"C", "C#", "D", "Eb", "E", "F", "F#", "G", "G#", "A", "Bb", "B"
		}
	},
	{
		KeySignature::CS,
		{
			"B#", "C#", "D", "D#", "E", "E#", "F#", "Fx", "G#", "A", "A#", "B"
		}
	},
	{
		KeySignature::DF,
		{
			"C", "Db", "EB", "Eb", "Fb", "F", "Gb", "G", "Ab", "BB", "Bb", "Cb"
		}
	},
	{
		KeySignature::D,
		{
			"C", "C#", "D", "Eb", "E", "F", "F#", "G", "G#", "A", "Bb", "B"
		}
	},
	{
		KeySignature::DS,
		{
			"B#", "C#", "Cx", "D#", "E", "E#", "F#", "Fx", "G#", "Gx", "A#", "B"
		}
	},
	{
		KeySignature::EF,
		{
			"C", "Db", "D", "Eb", "Fb", "F", "Gb", "G", "Ab", "A", "Bb", "Cb"
		}
	},
	{
		KeySignature::E,
		{
			"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
		}
	},
	{
		KeySignature::FF,
		{
			"DB", "Db", "EB", "Eb", "Fb", "GB", "Gb", "AB", "Ab", "BB", "Bb", "Cb"
		}
	},
	{
		KeySignature::ES,
		{
			"B#", "C#", "Cx", "D#", "Dx", "E#", "F#", "Fx", "G#", "Gx", "A#", "Ax"
		}
	},
	{
		KeySignature::F,
		{
			"C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B"
		}
	},
	{
		KeySignature::FS,
		{
			"B#", "C#", "D", "D#", "E", "E#", "F#", "G", "G#", "A", "A#", "B"
		}
	},
	{
		KeySignature::GF,
		{
			"C", "Db", "EB", "Eb", "Fb", "F", "Gb", "AB", "Ab", "BB", "Bb", "Cb"
		}
	},
	{
		KeySignature::G,
		{
			"C", "C#", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B"
		}
	},
	{
		KeySignature::GS,
		{
			"B#", "C#", "Cx", "D#", "E", "E#", "F#", "Fx", "G#", "A", "A#", "B"
		}
	},
	{
		KeySignature::AF,
		{
			"C", "Db", "D", "Eb", "Fb", "F", "Gb", "G", "Ab", "BB", "Bb", "Cb"
		}
	},
	{
		KeySignature::A,
		{
			"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "Ab", "B"
		}
	},
	{
		KeySignature::AS,
		{
			"B#", "C#", "Cx", "D#", "Dx", "E#", "F#", "G", "G#", "Gx", "A#", "B"
		}
	},
	{
		KeySignature::BF,
		{
			"C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "Cb"
		}
	},
	{
		KeySignature::B,
		{
			"C", "C#", "D", "D#", "E", "E#", "F#", "G", "G#", "A", "A#", "B"
		}
	},
	{
		KeySignature::CF,
		{
			"DB", "Db", "EB", "Eb", "Fb", "F", "Gb", "AB", "Ab", "BB", "Bb", "Cb"
		}
	},
	{
		KeySignature::BS,
		{
			"B#", "C#", "Cx", "D#", "Dx", "E#", "Ex", "Fx", "G#", "Gx", "A#", "Ax"
		}
	}
};

const std::unordered_map<KeySignature::Type, QStringList> NAMES_DE = {
	{
		KeySignature::C,
		{
			"C", "C#", "D", "Eb", "E", "F", "F#", "G", "G#", "A", "B", "H"
		}
	},
	{
		KeySignature::CS,
		{
			"H#", "C#", "D", "D#", "E", "E#", "F#", "Fx", "G#", "A", "A#", "H"
		}
	},
	{
		KeySignature::DF,
		{
			"C", "Db", "EB", "Eb", "Fb", "F", "Gb", "G", "Ab", "HB", "B", "Cb"
		}
	},
	{
		KeySignature::D,
		{
			"C", "C#", "D", "Eb", "E", "F", "F#", "G", "G#", "A", "B", "H"
		}
	},
	{
		KeySignature::DS,
		{
			"H#", "C#", "Cx", "D#", "E", "E#", "F#", "Fx", "G#", "Gx", "A#", "H"
		}
	},
	{
		KeySignature::EF,
		{
			"C", "Db", "D", "Eb", "Fb", "F", "Gb", "G", "Ab", "A", "B", "Cb"
		}
	},
	{
		KeySignature::E,
		{
			"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "H"
		}
	},
	{
		KeySignature::FF,
		{
			"DB", "Db", "EB", "Eb", "Fb", "GB", "Gb", "AB", "Ab", "HB", "B", "Cb"
		}
	},
	{
		KeySignature::ES,
		{
			"H#", "C#", "Cx", "D#", "Dx", "E#", "F#", "Fx", "G#", "Gx", "A#", "Ax"
		}
	},
	{
		KeySignature::F,
		{
			"C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "B", "H"
		}
	},
	{
		KeySignature::FS,
		{
			"H#", "C#", "D", "D#", "E", "E#", "F#", "G", "G#", "A", "A#", "H"
		}
	},
	{
		KeySignature::GF,
		{
			"C", "Db", "EB", "Eb", "Fb", "F", "Gb", "AB", "Ab", "HB", "B", "Cb"
		}
	},
	{
		KeySignature::G,
		{
			"C", "C#", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "B", "H"
		}
	},
	{
		KeySignature::GS,
		{
			"H#", "C#", "Cx", "D#", "E", "E#", "F#", "Fx", "G#", "A", "A#", "H"
		}
	},
	{
		KeySignature::AF,
		{
			"C", "Db", "D", "Eb", "Fb", "F", "Gb", "G", "Ab", "HB", "B", "Cb"
		}
	},
	{
		KeySignature::A,
		{
			"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "Ab", "H"
		}
	},
	{
		KeySignature::AS,
		{
			"H#", "C#", "Cx", "D#", "Dx", "E#", "F#", "G", "G#", "Gx", "A#", "H"
		}
	},
	{
		KeySignature::BF,
		{
			"C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "B", "Cb"
		}
	},
	{
		KeySignature::B,
		{
			"C", "C#", "D", "D#", "E", "E#", "F#", "G", "G#", "A", "A#", "H"
		}
	},
	{
		KeySignature::CF,
		{
			"DB", "Db", "EB", "Eb", "Fb", "F", "Gb", "AB", "Ab", "HB", "B", "Cb"
		}
	},
	{
		KeySignature::BS,
		{
			"H#", "C#", "Cx", "D#", "Dx", "E#", "Ex", "Fx", "G#", "Gx", "A#", "Ax"
		}
	}
};

std::unordered_map<NoteNotationSystem,
std::unordered_map<KeySignature::Type, QStringList>> NAMES = {
	{ NoteNotationSystem::ENGLISH, NAMES_EN },
	{ NoteNotationSystem::GERMAN, NAMES_DE }
};

std::unordered_map<KeySignature::Type, int> KEYS = {
	{ KeySignature::C, 0 },
	{ KeySignature::BS, 0 },
	{ KeySignature::CS, 1 },
	{ KeySignature::DF, 1 },
	{ KeySignature::D, 2 },
	{ KeySignature::DS, 3 },
	{ KeySignature::EF, 3 },
	{ KeySignature::E, 4 },
	{ KeySignature::FF, 4 },
	{ KeySignature::ES, 5 },
	{ KeySignature::F, 5 },
	{ KeySignature::FS, 6 },
	{ KeySignature::GF, 6 },
	{ KeySignature::G, 7 },
	{ KeySignature::GS, 8 },
	{ KeySignature::AF, 8 },
	{ KeySignature::A, 9 },
	{ KeySignature::AS, 10 },
	{ KeySignature::BF, 10 },
	{ KeySignature::B, 11 },
	{ KeySignature::CF, 11 }
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

QString NoteNameManager::getNoteName(int n, KeySignature::Type key) const
{
	return list_->at(key).at(n);
}

QString NoteNameManager::getNoteString(int noteNum, KeySignature::Type key) const
{
	return QString("%1%2").arg(list_->at(key).at(noteNum % 12), -2, QChar('-')).arg(noteNum / 12);
}

QString NoteNameManager::getKeyName(KeySignature::Type key) const
{
	return list_->at(key).at(KEYS.at(key));
}
