/*
 * SPDX-FileCopyrightText: 2025 Rerrah
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <QMessageBox>

namespace command_result_message_box
{
inline void showCommandInvokingErrorMessageBox(QWidget* parent)
{
	QMessageBox::critical(parent, QMessageBox::tr("Error"),
						  QMessageBox::tr("Failed to execute the command."),
						  QMessageBox::Ok, QMessageBox::Ok);
}

inline void showCommandRedoingErrorMessageBox(QWidget* parent)
{
	QMessageBox::critical(parent, QMessageBox::tr("Error"),
						  QMessageBox::tr("Failed to redo the command."),
						  QMessageBox::Ok, QMessageBox::Ok);
}

inline void showCommandUndoingErrorMessageBox(QWidget* parent)
{
	QMessageBox::critical(parent, QMessageBox::tr("Error"),
						  QMessageBox::tr("Failed to undo the command."),
						  QMessageBox::Ok, QMessageBox::Ok);
}
}
