#include "drop_detect_list_widget.hpp"

#include <QDropEvent>
#include <QMimeData>
#include <QByteArray>
#include <QDataStream>

DropDetectListWidget::DropDetectListWidget(QWidget* parent) : QListWidget(parent) {}

void DropDetectListWidget::dropEvent(QDropEvent* event)
{
	QListWidget::dropEvent(event);

	if (event->source() == this) {
		QListWidgetItem* tgt = itemAt(event->pos());
		if (tgt == nullptr) return;
		int tgtIdx = indexAt(event->pos()).row();

		int drpIdx;
		{	// Only 1 item stored
			QByteArray&& ary = event->mimeData()->data("application/x-qabstractitemmodeldatalist");
			QDataStream(&ary, QIODevice::ReadOnly) >> drpIdx;
		}

		emit itemDroppedAtItemIndex(drpIdx, tgtIdx);
	}
}
