#include "transpose_song_dialog.hpp"
#include "ui_transpose_song_dialog.h"
#include <QString>

TransposeSongDialog::TransposeSongDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::TransposeSongDialog)
{
	ui->setupUi(this);

	setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);

	for (int i = 0; i < 128; ++i) {
		auto item = new QListWidgetItem(QString("%1").arg(i, 2, 16, QChar('0')).toUpper());
		checks[i] = item;
		item->setCheckState(Qt::Unchecked);
		QObject::connect(ui->reversePushButton, &QPushButton::clicked,
						 this, [item] {
			item->setCheckState((item->checkState() == Qt::Unchecked) ? Qt::Checked : Qt::Unchecked);
		});
		QObject::connect(ui->clearPushButton, &QPushButton::clicked,
						 this, [item] { item->setCheckState(Qt::Unchecked); });
		ui->listWidget->addItem(item);
	}
}

TransposeSongDialog::~TransposeSongDialog()
{
	delete ui;
}

int TransposeSongDialog::getTransposeSeminotes() const
{
	return ui->spinBox->value();
}

std::vector<int> TransposeSongDialog::getExcludeInstruments() const
{
	std::vector<int> list;
	for (int i = 0; i < 128; ++i) {
		if (checks[i]->checkState() == Qt::Checked)
			list.push_back(i);
	}
	return list;
}
