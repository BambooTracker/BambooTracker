#include "effect_list_dialog.hpp"
#include "ui_effect_list_dialog.h"
#include <QTableWidgetItem>
#include "gui/effect_description.hpp"

EffectListDialog::EffectListDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::EffectListDialog)
{
	ui->setupUi(this);
	setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);

	ui->tableWidget->setColumnWidth(0, 50);
	ui->tableWidget->setColumnWidth(1, 100);

	addRow(EffectType::Arpeggio,
		   static_cast<int>(SoundSource::FM) | static_cast<int>(SoundSource::SSG));
	addRow(EffectType::PortamentoUp,
		   static_cast<int>(SoundSource::FM) | static_cast<int>(SoundSource::SSG));
	addRow(EffectType::PortamentoDown,
		   static_cast<int>(SoundSource::FM) | static_cast<int>(SoundSource::SSG));
	addRow(EffectType::TonePortamento,
		   static_cast<int>(SoundSource::FM) | static_cast<int>(SoundSource::SSG));
	addRow(EffectType::Vibrato,
		   static_cast<int>(SoundSource::FM) | static_cast<int>(SoundSource::SSG));
	addRow(EffectType::Tremolo,
		   static_cast<int>(SoundSource::FM) | static_cast<int>(SoundSource::SSG));
	addRow(EffectType::Pan,
		   static_cast<int>(SoundSource::FM) | static_cast<int>(SoundSource::DRUM));
	addRow(EffectType::VolumeSlide,
		   static_cast<int>(SoundSource::FM) | static_cast<int>(SoundSource::SSG));
	addRow(EffectType::PositionJump,
		   static_cast<int>(SoundSource::FM) | static_cast<int>(SoundSource::SSG) | static_cast<int>(SoundSource::DRUM));
	addRow(EffectType::SongEnd,
		   static_cast<int>(SoundSource::FM) | static_cast<int>(SoundSource::SSG) | static_cast<int>(SoundSource::DRUM));
	addRow(EffectType::PatternBreak,
		   static_cast<int>(SoundSource::FM) | static_cast<int>(SoundSource::SSG) | static_cast<int>(SoundSource::DRUM));
	addRow(EffectType::SpeedTempoChange,
		   static_cast<int>(SoundSource::FM) | static_cast<int>(SoundSource::SSG) | static_cast<int>(SoundSource::DRUM));
	addRow(EffectType::NoteDelay,
		   static_cast<int>(SoundSource::FM) | static_cast<int>(SoundSource::SSG) | static_cast<int>(SoundSource::DRUM));
	addRow(EffectType::AutoEnvelope,
		   static_cast<int>(SoundSource::SSG));
	addRow(EffectType::HardEnvHighPeriod,
		   static_cast<int>(SoundSource::SSG));
	addRow(EffectType::HardEnvLowPeriod,
		   static_cast<int>(SoundSource::SSG));
	addRow(EffectType::Groove,
		   static_cast<int>(SoundSource::FM) | static_cast<int>(SoundSource::SSG) | static_cast<int>(SoundSource::DRUM));
	addRow(EffectType::Detune,
		   static_cast<int>(SoundSource::FM) | static_cast<int>(SoundSource::SSG));
	addRow(EffectType::NoteSlideUp,
		   static_cast<int>(SoundSource::FM) | static_cast<int>(SoundSource::SSG));
	addRow(EffectType::NoteSlideDown,
		   static_cast<int>(SoundSource::FM) | static_cast<int>(SoundSource::SSG));
	addRow(EffectType::NoteCut,
		   static_cast<int>(SoundSource::FM) | static_cast<int>(SoundSource::SSG) | static_cast<int>(SoundSource::DRUM));
	addRow(EffectType::TransposeDelay,
		   static_cast<int>(SoundSource::FM) | static_cast<int>(SoundSource::SSG));
	addRow(EffectType::ToneNoiseMix,
		   static_cast<int>(SoundSource::SSG));
	addRow(EffectType::MasterVolume,
		   static_cast<int>(SoundSource::DRUM));
	addRow(EffectType::NoisePitch,
		   static_cast<int>(SoundSource::SSG));
	addRow(EffectType::ARControl,
		   static_cast<int>(SoundSource::FM));
	addRow(EffectType::Brightness,
		   static_cast<int>(SoundSource::FM));
	addRow(EffectType::DRControl,
		   static_cast<int>(SoundSource::FM));
	addRow(EffectType::FBControl,
		   static_cast<int>(SoundSource::FM));
	addRow(EffectType::VolumeDelay,
		   static_cast<int>(SoundSource::FM) | static_cast<int>(SoundSource::SSG) | static_cast<int>(SoundSource::DRUM));
	addRow(EffectType::MLControl,
		   static_cast<int>(SoundSource::FM));
	addRow(EffectType::RRControl,
		   static_cast<int>(SoundSource::FM));
	addRow(EffectType::TLControl,
		   static_cast<int>(SoundSource::FM));
}

EffectListDialog::~EffectListDialog()
{
	delete ui;
}

void EffectListDialog::addRow(EffectType effect, int flag)
{
	int row = ui->tableWidget->rowCount();
	ui->tableWidget->insertRow(row);
	ui->tableWidget->setItem(row, 0, new QTableWidgetItem(EffectDescription::getEffectFormat(effect)));
	ui->tableWidget->setRowHeight(row, ui->tableWidget->horizontalHeader()->height());
	QString type("");
	if (flag & static_cast<int>(SoundSource::FM)) type += "FM";
	if (flag & static_cast<int>(SoundSource::SSG)) type = type + (type.isEmpty() ? "" : ", ") + "SSG";
	if (flag & static_cast<int>(SoundSource::DRUM)) type = type + (type.isEmpty() ? "" : ", ") + "Drum";
	ui->tableWidget->setItem(row, 1, new QTableWidgetItem(type));
	ui->tableWidget->setItem(row, 2, new QTableWidgetItem(EffectDescription::getEffectDescription(effect)));
}
