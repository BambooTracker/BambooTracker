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
		QPoint pos = event->position().toPoint();
		QListWidgetItem* tgt = itemAt(pos);
		if (tgt == nullptr) return;
		int tgtIdx = indexAt(pos).row();

		int drpIdx;
		{	// Only 1 item stored
			QByteArray&& ary = event->mimeData()->data("application/x-qabstractitemmodeldatalist");
			QDataStream(&ary, QIODevice::ReadOnly) >> drpIdx;
		}

		emit itemDroppedAtItemIndex(drpIdx, tgtIdx);
	}
}
