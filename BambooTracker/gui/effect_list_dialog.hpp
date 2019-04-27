#ifndef EFFECT_LIST_DIALOG_HPP
#define EFFECT_LIST_DIALOG_HPP

#include <QDialog>
#include <QString>
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

	void addRow(QString effect, int flag, QString text);
};

#endif // EFFECT_LIST_DIALOG_HPP
