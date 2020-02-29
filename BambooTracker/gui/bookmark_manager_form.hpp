#ifndef BOOKMARK_MANAGER_FORM_HPP
#define BOOKMARK_MANAGER_FORM_HPP

#include <QWidget>
#include <QString>
#include <QListWidgetItem>
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

protected:
	bool eventFilter(QObject* watched, QEvent* event) override;

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
