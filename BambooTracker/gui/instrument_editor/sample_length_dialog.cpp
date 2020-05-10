#include "sample_length_dialog.hpp"
#include "ui_sample_length_dialog.h"

SampleLengthDialog::SampleLengthDialog(int len, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::SampleLengthDialog)
{
	ui->setupUi(this);

	setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);

	ui->spinBox->setValue(len);
}

SampleLengthDialog::~SampleLengthDialog()
{
	delete ui;
}

int SampleLengthDialog::getLength() const
{
	return ui->spinBox->value();
}
