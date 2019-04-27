#ifndef KEYBOARD_SHORTCUT_LIST_DIALOG_HPP
#define KEYBOARD_SHORTCUT_LIST_DIALOG_HPP

#include <QDialog>

namespace Ui {
	class KeyboardShortcutListDialog;
}

class KeyboardShortcutListDialog : public QDialog
{
	Q_OBJECT

public:
	explicit KeyboardShortcutListDialog(QWidget *parent = nullptr);
	~KeyboardShortcutListDialog();

private:
	Ui::KeyboardShortcutListDialog *ui;
};

#endif // KEYBOARD_SHORTCUT_LIST_DIALOG_HPP
