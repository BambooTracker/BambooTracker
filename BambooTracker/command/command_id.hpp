/*
 * Copyright (C) 2019-2020 Rerrah
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

enum CommandId : int
{
	// 0x1*: Instrument list
	AddInstrument			= 0x10,
	RemoveInstrument		= 0x11,
	ChangeInstrumentName	= 0x12,
	CloneInstrument			= 0x13,
	DeepCloneInstrument		= 0x14,
	SwapInstruments			= 0x15,

	// 0x2*, 0x3*: Pattern editor
	SetKeyOnToStep						= 0x20,
	SetKeyOffToStep						= 0x21,
	EraseStep							= 0x22,
	SetInstrumentInStep					= 0x23,
	EraseInstrumentInStep				= 0x24,
	SetVolumeToStep						= 0x25,
	EraseVolumeInStep					= 0x26,
	SetEffectIDToStep					= 0x27,
	EraseEffectInStep					= 0x28,
	SetEffectValueToStep				= 0x29,
	EraseEffectValueInStep				= 0x2a,
	InsertStep							= 0x2b,
	DeletePreviousStep					= 0x2c,
	PasteCopiedDataToPattern			= 0x2d,
	EraseCellsInPattern					= 0x2e,
	PasteMixCopiedDataToPattern			= 0x2f,
	TransposeNoteInPattern				= 0x30,
	ChangeValuesInPattern				= 0x31,
	ExpandPattern						= 0x34,
	ShrinkPattern						= 0x35,
	SetEchoBufferAccess					= 0x36,
	InterpolatePattern					= 0x37,
	ReversePattern						= 0x38,
	ReplaceInstrumentInPattern			= 0x39,
	PasteOverwriteCopiedDataToPattern	= 0x3a,
	PasteInsertCopiedDataToPattern		= 0x3b,

	// 0x4*: Order list
	SetPatternToOrder		= 0x40,
	InsertOrderBelow		= 0x41,
	DeleteOrder				= 0x42,
	PasteCopiedDataToOrder	= 0x43,
	DuplicateOrder			= 0x44,
	MoveOrder				= 0x45,
	ClonePatterns			= 0x46,
	CloneOrder				= 0x47
};
