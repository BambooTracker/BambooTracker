#include "keyboard_shortcut_list_dialog.hpp"
#include "ui_keyboard_shortcut_list_dialog.h"

KeyboardShortcutListDialog::KeyboardShortcutListDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::KeyboardShortcutListDialog)
{
	ui->setupUi(this);
	setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);
}

KeyboardShortcutListDialog::~KeyboardShortcutListDialog()
{
	delete ui;
}
