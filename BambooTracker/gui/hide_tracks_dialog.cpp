#include "hide_tracks_dialog.hpp"
#include "ui_hide_tracks_dialog.h"
#include <QListWidgetItem>
#include "song.hpp"
#include "gui/gui_util.hpp"

HideTracksDialog::HideTracksDialog(const SongStyle& style, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::HideTracksDialog),
	loopCheck_(false)
{
	ui->setupUi(this);

	setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);

	for (const auto& attrib : style.trackAttribs) {
		auto item = new QListWidgetItem(getTrackName(style.type, attrib.source, attrib.channelInSource), ui->listWidget);
		item->setData(Qt::UserRole, attrib.number);	// Track numbers are sorted
		item->setCheckState(Qt::Checked);
	}
	QObject::connect(ui->listWidget, &QListWidget::itemChanged,
					 this, [&](QListWidgetItem* item) {
		if (item->checkState() == Qt::Unchecked) {
			if (checkCounter_ == 1) {
				loopCheck_ = true;
				item->setCheckState(Qt::Checked);
			}
			else {
				--checkCounter_;
			}
		}
		else {
			if (loopCheck_) {
				loopCheck_ = false;
				return;
			}
			++checkCounter_;
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
