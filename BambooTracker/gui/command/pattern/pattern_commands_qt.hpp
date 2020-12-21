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

#ifndef PATTERN_COMMANDS_QT_HPP
#define PATTERN_COMMANDS_QT_HPP

#include "pattern_editor_common_qt_command.hpp"

using ChangeValuesInPatternQtCommand = gui_command_impl::PatternEditorCommonQtCommandRedrawAll<CommandId::ChangeValuesInPattern>;
using DeletePreviousStepQtCommand = gui_command_impl::PatternEditorCommonQtCommandRedrawAll<CommandId::DeletePreviousStep>;
using EraseCellsInPatternQtCommand = gui_command_impl::PatternEditorCommonQtCommandRedrawAll<CommandId::EraseCellsInPattern>;
using EraseEffectInStepQtCommand = gui_command_impl::PatternEditorCommonQtCommandRedrawAll<CommandId::EraseEffectInStep>;
using EraseEffectValueInStepQtCommand = gui_command_impl::PatternEditorCommonQtCommandRedrawAll<CommandId::EraseEffectValueInStep>;
using EraseInstrumentInStepQtCommand = gui_command_impl::PatternEditorCommonQtCommandRedrawText<CommandId::EraseInstrumentInStep>;
using EraseStepQtCommand = gui_command_impl::PatternEditorCommonQtCommandRedrawAll<CommandId::EraseStep>;
using EraseVolumeInStepQtCommand = gui_command_impl::PatternEditorCommonQtCommandRedrawText<CommandId::EraseVolumeInStep>;
using ExpandPatternQtCommand = gui_command_impl::PatternEditorCommonQtCommandRedrawAll<CommandId::ExpandPattern>;
using InsertStepQtCommand = gui_command_impl::PatternEditorCommonQtCommandRedrawAll<CommandId::InsertStep>;
using InterpolatePatternQtCommand = gui_command_impl::PatternEditorCommonQtCommandRedrawText<CommandId::InterpolatePattern>;
using PasteCopiedDataToPatternQtCommand = gui_command_impl::PatternEditorCommonQtCommandRedrawAll<CommandId::PasteCopiedDataToPattern>;
using PasteInsertCopiedDataToPatternQtCommand = gui_command_impl::PatternEditorCommonQtCommandRedrawAll<CommandId::PasteInsertCopiedDataToPattern>;
using PasteMixCopiedDataToPatternQtCommand = gui_command_impl::PatternEditorCommonQtCommandRedrawAll<CommandId::PasteMixCopiedDataToPattern>;
using PasteOverwriteCopiedDataToPatternQtCommand = gui_command_impl::PatternEditorCommonQtCommandRedrawAll<CommandId::PasteOverwriteCopiedDataToPattern>;
using ReplaceInstrumentInPatternQtCommand = gui_command_impl::PatternEditorCommonQtCommandRedrawText<CommandId::ReplaceInstrumentInPattern>;
using ReversePatternQtCommand = gui_command_impl::PatternEditorCommonQtCommandRedrawAll<CommandId::ReversePattern>;
using SetEchoBufferAccessQtCommand = gui_command_impl::PatternEditorCommonQtCommandRedrawText<CommandId::SetEchoBufferAccess>;
using SetEffectIDToStepQtCommand = gui_command_impl::PatternEditorEntryQtCommandRedrawAll<CommandId::SetEffectIDToStep>;
using SetEffectValueToStepQtCommand = gui_command_impl::PatternEditorEntryQtCommandRedrawText<CommandId::SetEffectValueToStep>;
using SetInstrumentToStepQtCommand = gui_command_impl::PatternEditorEntryQtCommandRedrawText<CommandId::SetInstrumentInStep>;
using SetKeyOffToStepQtCommand = gui_command_impl::PatternEditorCommonQtCommandRedrawText<CommandId::SetKeyOffToStep>;
using SetKeyOnToStepQtCommand = gui_command_impl::PatternEditorCommonQtCommandRedrawText<CommandId::SetKeyOnToStep>;
using SetVolumeToStepQtCommand = gui_command_impl::PatternEditorEntryQtCommandRedrawText<CommandId::SetVolumeToStep>;
using ShrinkPatternQtCommand = gui_command_impl::PatternEditorCommonQtCommandRedrawAll<CommandId::ShrinkPattern>;
using TransposeNoteInPatternQtCommand = gui_command_impl::PatternEditorCommonQtCommandRedrawText<CommandId::TransposeNoteInPattern>;

#endif // PATTERN_COMMANDS_QT_HPP
