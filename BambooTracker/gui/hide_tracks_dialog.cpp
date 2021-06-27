/*
 * Copyright (C) 2020-2021 Rerrah
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

#include "hide_tracks_dialog.hpp"
#include "ui_hide_tracks_dialog.h"
#include <algorithm>
#include <QListWidgetItem>
#include "song.hpp"
#include "gui/gui_utils.hpp"

HideTracksDialog::HideTracksDialog(const SongStyle& style, const std::vector<int>& tracks, QWidget *parent)
	: QDialog(parent),
	  ui(new Ui::HideTracksDialog)
{
	ui->setupUi(this);

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	for (const auto& attrib : style.trackAttribs) {
		auto item = new QListWidgetItem(gui_utils::getTrackName(style.type, attrib.source, attrib.channelInSource), ui->listWidget);
		int n = attrib.number;
		item->setData(Qt::UserRole, n);	// Track numbers are sorted
		bool exists = std::any_of(tracks.begin(), tracks.end(), [n](int t) { return n == t; });
		item->setCheckState(exists ? Qt::Checked : Qt::Unchecked);
	}
	QObject::connect(ui->listWidget, &QListWidget::itemChanged,
					 this, [&](QListWidgetItem* item) {
		if (item->checkState() == Qt::Unchecked) {
			--checkCounter_;
			if (!checkCounter_) ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
		}
		else {
			++checkCounter_;
			ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
		}
	});
	checkCounter_ = style.trackAttribs.size();
}

HideTracksDialog::~HideTracksDialog()
{
	delete ui;
}

std::vector<int> HideTracksDialog::getVisibleTracks() const
{
	std::vector<int> tracks;
	for (int i = 0; i < ui->listWidget->count(); ++i) {
		auto item = ui->listWidget->item(i);
		if (item->checkState() == Qt::Checked)
			tracks.push_back(item->data(Qt::UserRole).toInt());
	}
	return tracks;
}

void HideTracksDialog::on_reversePushButton_clicked()
{
	for (int i = 0; i < ui->listWidget->count(); ++i) {
		auto item = ui->listWidget->item(i);
		item->setCheckState((item->checkState() == Qt::Checked) ? Qt::Unchecked : Qt::Checked);
	}
}

void HideTracksDialog::on_checkAllPushButton_clicked()
{
	for (int i = 0; i < ui->listWidget->count(); ++i) {
		auto item = ui->listWidget->item(i);
		item->setCheckState(Qt::Checked);
	}
}
