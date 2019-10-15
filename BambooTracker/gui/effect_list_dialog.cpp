#include "effect_list_dialog.hpp"
#include "ui_effect_list_dialog.h"
#include <QTableWidgetItem>

EffectListDialog::EffectListDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::EffectListDialog)
{
	ui->setupUi(this);
	setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);

	ui->tableWidget->setColumnWidth(0, 50);
	ui->tableWidget->setColumnWidth(1, 100);

	addRow("00xy",
		   static_cast<int>(SoundSource::FM) | static_cast<int>(SoundSource::SSG),
		   tr("Arpeggio, x: 2nd note (0-F), y: 3rd note (0-F)"));
	addRow("01xx",
		   static_cast<int>(SoundSource::FM) | static_cast<int>(SoundSource::SSG),
		   tr("Portamento up, xx: depth (00-FF)"));
	addRow("02xx",
		   static_cast<int>(SoundSource::FM) | static_cast<int>(SoundSource::SSG),
		   tr("Portamento down, xx: depth (00-FF)"));
	addRow("03xx",
		   static_cast<int>(SoundSource::FM) | static_cast<int>(SoundSource::SSG),
		   tr("Tone portamento, xx: depth (00-FF)"));
	addRow("04xy",
		   static_cast<int>(SoundSource::FM) | static_cast<int>(SoundSource::SSG),
		   tr("Vibrato, x: period (0-F), y: depth (0-F)"));
	addRow("07xx",
		   static_cast<int>(SoundSource::FM) | static_cast<int>(SoundSource::SSG),
		   tr("Tremolo, x: period (0-F), y: depth (0-F)"));
	addRow("08xx",
		   static_cast<int>(SoundSource::FM) | static_cast<int>(SoundSource::Drum),
		   tr("Pan, xx: 00 = no sound, 01 = right, 02 = left, 03 = center"));
	addRow("0Axy",
		   static_cast<int>(SoundSource::FM) | static_cast<int>(SoundSource::SSG),
		   tr("Volume slide, x: up (0-F), y: down (0-F)"));
	addRow("0Bxx",
		   static_cast<int>(SoundSource::FM) | static_cast<int>(SoundSource::SSG) | static_cast<int>(SoundSource::Drum),
		   tr("Jump to beginning of order xx"));
	addRow("0Cxx",
		   static_cast<int>(SoundSource::FM) | static_cast<int>(SoundSource::SSG) | static_cast<int>(SoundSource::Drum),
		   tr("End of song"));
	addRow("0Dxx",
		   static_cast<int>(SoundSource::FM) | static_cast<int>(SoundSource::SSG) | static_cast<int>(SoundSource::Drum),
		   tr("Jump to step xx of next order"));
	addRow("0Fxx",
		   static_cast<int>(SoundSource::FM) | static_cast<int>(SoundSource::SSG) | static_cast<int>(SoundSource::Drum),
		   tr("Change speed (xx: 00-1F), change tempo (xx: 20-FF)"));
	addRow("0Gxx",
		   static_cast<int>(SoundSource::FM) | static_cast<int>(SoundSource::SSG) | static_cast<int>(SoundSource::Drum),
		   tr("Note delay, xx: count (00-FF)"));
	addRow("0Ixx",
		   static_cast<int>(SoundSource::SSG),
		   tr("Hardware envelope frequency 1, xx: high byte (00-FF)"));
	addRow("0Jxx",
		   static_cast<int>(SoundSource::SSG),
		   tr("Hardware envelope frequency 2, xx: low byte (00-FF)"));
	addRow("0Oxx",
		   static_cast<int>(SoundSource::FM) | static_cast<int>(SoundSource::SSG) | static_cast<int>(SoundSource::Drum),
		   tr("Set groove xx"));
	addRow("0Pxx",
		   static_cast<int>(SoundSource::FM) | static_cast<int>(SoundSource::SSG),
		   tr("Detune, xx: pitch (00-FF)"));
	addRow("0Qxy",
		   static_cast<int>(SoundSource::FM) | static_cast<int>(SoundSource::SSG),
		   tr("Note slide up, x: count (0-F), y: seminote (0-F)"));
	addRow("0Rxy",
		   static_cast<int>(SoundSource::FM) | static_cast<int>(SoundSource::SSG),
		   tr("Note slide down, x: count (0-F), y: seminote (0-F)"));
	addRow("07xx",
		   static_cast<int>(SoundSource::FM) | static_cast<int>(SoundSource::SSG),
		   tr("Tremolo, x: period (0-F), y: depth (0-F)"));
	addRow("0Sxx",
		   static_cast<int>(SoundSource::FM) | static_cast<int>(SoundSource::SSG) | static_cast<int>(SoundSource::Drum),
		   tr("Note cut, xx: count (01-FF)"));
	addRow("0Txy",
		   static_cast<int>(SoundSource::FM) | static_cast<int>(SoundSource::SSG),
		   tr("Transpose delay, x: count (1-7: up, 9-F: down), y: seminote (0-F)"));
	addRow("0Vxx",
		   static_cast<int>(SoundSource::SSG),
		   tr("Tone/Noise mix, xx: 00 = no sound, 01 = tone, 02 = noise, 03 = tone & noise"));
	addRow("0Vxx",
		   static_cast<int>(SoundSource::Drum),
		   tr("Master volume, xx: volume (00-3F)"));
	addRow("0Wxx",
		   static_cast<int>(SoundSource::SSG),
		   tr("Noise frequency, xx: frequency (00-1F)"));
	addRow("Mxyy",
		   static_cast<int>(SoundSource::FM) | static_cast<int>(SoundSource::SSG) | static_cast<int>(SoundSource::Drum),
		   tr("Volume delay, x: count (1-F), yy: volume (00-FF)"));
}

EffectListDialog::~EffectListDialog()
{
	delete ui;
}

void EffectListDialog::addRow(QString effect, int flag, QString text)
{
	int row = ui->tableWidget->rowCount();
	ui->tableWidget->insertRow(row);
	ui->tableWidget->setItem(row, 0, new QTableWidgetItem(effect));
	ui->tableWidget->setRowHeight(row, ui->tableWidget->horizontalHeader()->height());
	QString type("");
	if (flag & static_cast<int>(SoundSource::FM)) type += "FM";
	if (flag & static_cast<int>(SoundSource::SSG)) type = type + (type.isEmpty() ? "" : ", ") + "SSG";
	if (flag & static_cast<int>(SoundSource::Drum)) type = type + (type.isEmpty() ? "" : ", ") + "Drum";
	ui->tableWidget->setItem(row, 1, new QTableWidgetItem(type));
	ui->tableWidget->setItem(row, 2, new QTableWidgetItem(text));
}
