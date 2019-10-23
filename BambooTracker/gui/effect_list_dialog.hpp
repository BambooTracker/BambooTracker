#ifndef EFFECT_LIST_DIALOG_HPP
#define EFFECT_LIST_DIALOG_HPP

#include <QDialog>
#include <QString>
#include "effect.hpp"
#include "misc.hpp"

namespace Ui {
	class EffectListDialog;
}

class EffectListDialog : public QDialog
{
	Q_OBJECT

public:
	explicit EffectListDialog(QWidget *parent = nullptr);
	~EffectListDialog();

private:
	Ui::EffectListDialog *ui;

	void addRow(EffectType effect, int flag);
};

#endif // EFFECT_LIST_DIALOG_HPP
