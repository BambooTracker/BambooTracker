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

	addRow(EffectType::Arpeggio, { SoundSource::FM, SoundSource::SSG, SoundSource::ADPCM });
	addRow(EffectType::PortamentoUp, { SoundSource::FM, SoundSource::SSG, SoundSource::ADPCM });
	addRow(EffectType::PortamentoDown, { SoundSource::FM, SoundSource::SSG, SoundSource::ADPCM });
	addRow(EffectType::TonePortamento, { SoundSource::FM, SoundSource::SSG, SoundSource::ADPCM });
	addRow(EffectType::Vibrato, { SoundSource::FM, SoundSource::SSG, SoundSource::ADPCM });
	addRow(EffectType::Tremolo, { SoundSource::FM, SoundSource::SSG, SoundSource::ADPCM });
	addRow(EffectType::Pan, { SoundSource::FM, SoundSource::RHYTHM, SoundSource::ADPCM });
	addRow(EffectType::VolumeSlide, { SoundSource::FM, SoundSource::SSG, SoundSource::ADPCM });
	addRow(EffectType::PositionJump, { SoundSource::FM, SoundSource::SSG, SoundSource::RHYTHM, SoundSource::ADPCM });
	addRow(EffectType::SongEnd, { SoundSource::FM, SoundSource::SSG, SoundSource::RHYTHM, SoundSource::ADPCM });
	addRow(EffectType::PatternBreak, { SoundSource::FM, SoundSource::SSG, SoundSource::RHYTHM, SoundSource::ADPCM });
	addRow(EffectType::SpeedTempoChange, { SoundSource::FM, SoundSource::SSG, SoundSource::RHYTHM, SoundSource::ADPCM });
	addRow(EffectType::NoteDelay, { SoundSource::FM, SoundSource::SSG, SoundSource::RHYTHM, SoundSource::ADPCM });
	addRow(EffectType::AutoEnvelope, { SoundSource::SSG });
	addRow(EffectType::HardEnvHighPeriod, { SoundSource::SSG });
	addRow(EffectType::HardEnvLowPeriod, { SoundSource::SSG });
	addRow(EffectType::Groove, { SoundSource::FM, SoundSource::SSG, SoundSource::RHYTHM, SoundSource::ADPCM });
	addRow(EffectType::Detune, { SoundSource::FM, SoundSource::SSG, SoundSource::ADPCM });
	addRow(EffectType::NoteSlideUp, { SoundSource::FM, SoundSource::SSG, SoundSource::ADPCM });
	addRow(EffectType::NoteSlideDown, { SoundSource::FM, SoundSource::SSG, SoundSource::ADPCM });
	addRow(EffectType::NoteCut, { SoundSource::FM, SoundSource::SSG, SoundSource::RHYTHM, SoundSource::ADPCM });
	addRow(EffectType::TransposeDelay, { SoundSource::FM, SoundSource::SSG, SoundSource::ADPCM });
	addRow(EffectType::ToneNoiseMix, { SoundSource::SSG });
	addRow(EffectType::MasterVolume, { SoundSource::RHYTHM });
	addRow(EffectType::NoisePitch, { SoundSource::SSG });
	addRow(EffectType::ARControl, { SoundSource::FM });
	addRow(EffectType::Brightness, { SoundSource::FM });
	addRow(EffectType::DRControl, { SoundSource::FM });
	addRow(EffectType::FBControl, { SoundSource::FM });
	addRow(EffectType::VolumeDelay, { SoundSource::FM, SoundSource::SSG, SoundSource::RHYTHM, SoundSource::ADPCM });
	addRow(EffectType::MLControl, { SoundSource::FM });
	addRow(EffectType::RRControl, { SoundSource::FM });
	addRow(EffectType::TLControl, { SoundSource::FM });
}

EffectListDialog::~EffectListDialog()
{
	delete ui;
}

void EffectListDialog::addRow(EffectType effect, std::unordered_set<SoundSource> types)
{
	int row = ui->tableWidget->rowCount();
	ui->tableWidget->insertRow(row);
	ui->tableWidget->setItem(row, 0, new QTableWidgetItem(EffectDescription::getEffectFormat(effect)));
	ui->tableWidget->setRowHeight(row, ui->tableWidget->horizontalHeader()->height());
	QString typeStr("");
	if (types.count(SoundSource::FM)) typeStr += "FM";
	if (types.count(SoundSource::SSG)) typeStr = typeStr + (typeStr.isEmpty() ? "" : ", ") + "SSG";
	if (types.count(SoundSource::RHYTHM)) typeStr = typeStr + (typeStr.isEmpty() ? "" : ", ") + tr("Rhythm");
	if (types.count(SoundSource::ADPCM)) typeStr = typeStr + (typeStr.isEmpty() ? "" : ", ") + "ADPCM";
	ui->tableWidget->setItem(row, 1, new QTableWidgetItem(typeStr));
	ui->tableWidget->setItem(row, 2, new QTableWidgetItem(EffectDescription::getEffectDescription(effect)));
}
