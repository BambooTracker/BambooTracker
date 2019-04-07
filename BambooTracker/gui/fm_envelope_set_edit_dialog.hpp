#ifndef FM_ENVELOPE_SET_EDIT_DIALOG_HPP
#define FM_ENVELOPE_SET_EDIT_DIALOG_HPP

#include <QDialog>
#include <QComboBox>
#include <vector>
#include "misc.hpp"

namespace Ui {
	class FMEnvelopeSetEditDialog;
}

class FMEnvelopeSetEditDialog : public QDialog
{
	Q_OBJECT

public:
	explicit FMEnvelopeSetEditDialog(std::vector<FMEnvelopeTextType> set, QWidget *parent = nullptr);
	~FMEnvelopeSetEditDialog();
	std::vector<FMEnvelopeTextType> getSet();

private slots:
	void on_upToolButton_clicked();
	void on_downToolButton_clicked();
	void on_addPushButton_clicked();
	void on_removePushButton_clicked();
	void on_treeWidget_itemSelectionChanged();

private:
	Ui::FMEnvelopeSetEditDialog *ui;

	void swapset(int aboveRow, int belowRow);
	void insertRow(int row, FMEnvelopeTextType type);
	QComboBox* makeCombobox();
	void alignTreeOn1stItemChanged();
};

#endif // FM_ENVELOPE_SET_EDIT_DIALOG_HPP
