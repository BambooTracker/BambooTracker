/*
 * Copyright (C) 2019-2021 Rerrah
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "effect_list_dialog.hpp"
#include "ui_effect_list_dialog.h"
#include <QTableWidgetItem>
#include <QString>
#include "gui/effect_description.hpp"

EffectListDialog::EffectListDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::EffectListDialog)
{
	ui->setupUi(this);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

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
	addRow(EffectType::Retrigger, { SoundSource::FM, SoundSource::SSG, SoundSource::RHYTHM, SoundSource::ADPCM });
	addRow(EffectType::Groove, { SoundSource::FM, SoundSource::SSG, SoundSource::RHYTHM, SoundSource::ADPCM });
	addRow(EffectType::Detune, { SoundSource::FM, SoundSource::SSG, SoundSource::ADPCM });
	addRow(EffectType::NoteSlideUp, { SoundSource::FM, SoundSource::SSG, SoundSource::ADPCM });
	addRow(EffectType::NoteSlideDown, { SoundSource::FM, SoundSource::SSG, SoundSource::ADPCM });
	addRow(EffectType::NoteCut, { SoundSource::FM, SoundSource::SSG, SoundSource::RHYTHM, SoundSource::ADPCM });
	addRow(EffectType::TransposeDelay, { SoundSource::FM, SoundSource::SSG, SoundSource::ADPCM });
	addRow(EffectType::ToneNoiseMix, { SoundSource::SSG });
	addRow(EffectType::MasterVolume, { SoundSource::RHYTHM });
	addRow(EffectType::NoisePitch, { SoundSource::SSG });
	addRow(EffectType::RegisterAddress0, { SoundSource::FM, SoundSource::SSG, SoundSource::RHYTHM, SoundSource::ADPCM });
	addRow(EffectType::RegisterAddress1, { SoundSource::FM, SoundSource::SSG, SoundSource::RHYTHM, SoundSource::ADPCM });
	addRow(EffectType::RegisterValue, { SoundSource::FM, SoundSource::SSG, SoundSource::RHYTHM, SoundSource::ADPCM });
	addRow(EffectType::ARControl, { SoundSource::FM });
	addRow(EffectType::Brightness, { SoundSource::FM });
	addRow(EffectType::DRControl, { SoundSource::FM });
	addRow(EffectType::EnvelopeReset, { SoundSource::FM });
	addRow(EffectType::FBControl, { SoundSource::FM });
	addRow(EffectType::FineDetune, { SoundSource::FM, SoundSource::SSG, SoundSource::ADPCM });
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
