#include "hide_tracks_dialog.hpp"
#include "ui_hide_tracks_dialog.h"
#include <algorithm>
#include <QListWidgetItem>
#include "song.hpp"
#include "gui/gui_util.hpp"

HideTracksDialog::HideTracksDialog(const SongStyle& style, const std::vector<int>& tracks, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::HideTracksDialog)
{
	ui->setupUi(this);

	setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);

	for (const auto& attrib : style.trackAttribs) {
		auto item = new QListWidgetItem(getTrackName(style.type, attrib.source, attrib.channelInSource), ui->listWidget);
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
