/*
 * Copyright (C) 2018-2021 Rerrah
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

#include "wave_export_settings_dialog.hpp"
#include "ui_wave_export_settings_dialog.h"
#include <algorithm>
#include <QListWidgetItem>
#include "gui/gui_utils.hpp"
#include "song.hpp"

WaveExportSettingsDialog::WaveExportSettingsDialog(const std::vector<int> defUnmutes, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::WaveExportSettingsDialog)
{
	ui->setupUi(this);

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	ui->sampleRateComboBox->addItem("44100Hz", 44100);
	ui->sampleRateComboBox->addItem("48000Hz", 48000);
	ui->sampleRateComboBox->addItem("55466Hz", 55466);

	struct Pair
	{
		SoundSource src;
		int count;
	};
	static const QList<Pair> SRC_NUMS = {
		{ SoundSource::FM, 6 }, { SoundSource::SSG, 3 }, { SoundSource::RHYTHM, 6 }, { SoundSource::ADPCM, 1 }
	};

	int tmpT = 0;
	for (const auto& pair : SRC_NUMS) {
		for (int i = 0; i < pair.count; ++i, ++tmpT) {
			auto item = new QListWidgetItem(gui_utils::getTrackName(SongType::Standard, pair.src, i), ui->tracksListWidget);
			item->setCheckState(std::any_of(defUnmutes.begin(), defUnmutes.end(), [tmpT](const int t) { return t == tmpT; })
								? Qt::Checked : Qt::Unchecked);
		}
	}
}

WaveExportSettingsDialog::~WaveExportSettingsDialog()
{
	delete ui;
}

int WaveExportSettingsDialog::getSampleRate() const
{
	return ui->sampleRateComboBox->currentData().toInt();
}

int WaveExportSettingsDialog::getLoopCount() const
{
	return ui->loopSpinBox->value();
}

std::vector<int> WaveExportSettingsDialog::getSoloExportTracks() const
{
	if (!ui->tracksGroupBox->isChecked()) return {};

	std::vector<int> tracks;
	for (int i = 0; i < ui->tracksListWidget->count(); ++i) {
		if (ui->tracksListWidget->item(i)->checkState() == Qt::Checked)
			tracks.push_back(i);
	}
	return tracks;
}

void WaveExportSettingsDialog::on_reversePushButton_clicked()
{
	for (int i = 0; i < ui->tracksListWidget->count(); ++i) {
		auto item = ui->tracksListWidget->item(i);
		item->setCheckState((item->checkState() == Qt::Checked) ? Qt::Unchecked : Qt::Checked);
	}
}

void WaveExportSettingsDialog::on_allPushButton_clicked()
{
	for (int i = 0; i < ui->tracksListWidget->count(); ++i) {
		auto item = ui->tracksListWidget->item(i);
		item->setCheckState(Qt::Checked);
	}
}
