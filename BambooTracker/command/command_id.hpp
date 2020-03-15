#pragma once

enum CommandId : int
{
	// 0x1*: Instrument list
	AddInstrument			= 0x10,
	RemoveInstrument		= 0x11,
	ChangeInstrumentName	= 0x12,
	CloneInstrument			= 0x13,
	DeepCloneInstrument		= 0x14,

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
	TransposeNoteKeyInPattern			= 0x30,
	IncreaseNoteOctaveInPattern			= 0x32,
	DecreaseNoteOctaveInPattern			= 0x33,
	ExpandPattern						= 0x34,
	ShrinkPattern						= 0x35,
	SetEchoBufferAccess					= 0x36,
	InterpolatePattern					= 0x37,
	ReversePattern						= 0x38,
	ReplaceInstrumentInPattern			= 0x39,
	PasteOverwriteCopiedDataToPattern	= 0x3a,

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
