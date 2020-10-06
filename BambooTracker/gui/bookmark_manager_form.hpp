/*
 * Copyright (C) 2020 Rerrah
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

#ifndef BOOKMARK_MANAGER_FORM_HPP
#define BOOKMARK_MANAGER_FORM_HPP

#include <QWidget>
#include <QString>
#include <QListWidgetItem>
#include <QShortcut>
#include <memory>
#include "bamboo_tracker.hpp"

namespace Ui {
	class BookmarkManagerForm;
}

class BookmarkManagerForm : public QWidget
{
	Q_OBJECT

public:
	BookmarkManagerForm(std::weak_ptr<BambooTracker> core, bool showHex,
						QWidget *parent = nullptr);
	~BookmarkManagerForm() override;

signals:
	void positionJumpRequested(int order, int step);
	void modified();

public slots:
	void onCurrentSongNumberChanged();
	void onConfigurationChanged(bool showHex);
	void onBookmarkToggleRequested(int order, int step);
	void onBookmarkJumpRequested(bool toNext, int order, int step);

private slots:
	void on_createPushButton_clicked();
	void on_removePushButton_clicked();
	void on_clearPushButton_clicked();
	void on_upToolButton_clicked();
	void on_downToolButton_clicked();
	void on_positionPushButton_clicked();
	void on_namePushButton_clicked();
	void on_listWidget_currentRowChanged(int currentRow);
	void on_updatePushButton_clicked();
	void on_listWidget_itemDoubleClicked(QListWidgetItem *item);

private:
	Ui::BookmarkManagerForm *ui;
	std::weak_ptr<BambooTracker> bt_;

	int curSong_;
	int numWidth_, numBase_;

	std::unique_ptr<QShortcut> insSc_, delSc_, mvUpSc_, mvDnSc_;

	void initList();

	inline void setNumberSettings(bool showHex)
	{
		if (showHex) {
			numWidth_ = 2;
			numBase_ = 16;
		}
		else {
			numWidth_ = 3;
			numBase_ = 10;
		}
	}

	void addBookmark(QString name, int order, int step, bool onlyUi = false);
	void removeBookmark(int i);
	QString createText(QString name, int order, int step);
	void sortList(bool byPos);
};

#endif // BOOKMARK_MANAGER_FORM_HPP
