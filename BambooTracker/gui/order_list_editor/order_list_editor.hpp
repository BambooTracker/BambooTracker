#ifndef ORDER_LIST_EDITOR_HPP
#define ORDER_LIST_EDITOR_HPP

#include <QFrame>
#include <QUndoStack>
#include <QEvent>
#include <memory>
#include "bamboo_tracker.hpp"
#include "configuration.hpp"
#include "gui/color_palette.hpp"

namespace Ui {
	class OrderListEditor;
}

class OrderListEditor : public QFrame
{
	Q_OBJECT

public:
	explicit OrderListEditor(QWidget *parent = nullptr);
	~OrderListEditor() override;

	void setCore(std::shared_ptr<BambooTracker> core);
	void setCommandStack(std::weak_ptr<QUndoStack> stack);
	void setConfiguration(std::shared_ptr<Configuration> config);
	void setColorPallete(std::shared_ptr<ColorPalette> palette);

	void changeEditable();
	void updatePositionByOrderUpdate(bool isFirstUpdate);

	void copySelectedCells();
	void deleteOrder();
	void insertOrderBelow();

	void freeze();
	void unfreeze();

	QString getHeaderFont() const;
	int getHeaderFontSize() const;
	QString getRowsFont() const;
	int getRowsFontSize() const;
	void setFonts(QString headerFont, int headerSize, QString rowsFont, int rowsSize);

	void setHorizontalScrollMode(bool cellBased);

signals:
	void currentTrackChanged(int num);
	void currentOrderChanged(int num);

	void orderEdited();

	void focusIn();
	void focusOut();
	void selected(bool isSelected);
	void returnPressed();

public slots:
	void setCurrentTrack(int num);
	void setCurrentOrder(int num, int max);
	void onSongLoaded();

	void onPastePressed();
	/// 0: None
	/// 1: All
	/// 2: Row
	/// 3: Column
	/// 4: Pattern
	/// 5: Order
	void onSelectPressed(int type);
	void onDuplicatePressed();
	void onMoveOrderPressed(bool isUp);
	void onClonePatternsPressed();
	void onCloneOrderPressed();
	void onFollowModeChanged();
	void onStoppedPlaySong();

protected:
	bool eventFilter(QObject *watched, QEvent *event) override;
	void showEvent(QShowEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;

private:
	Ui::OrderListEditor *ui;
	std::shared_ptr<BambooTracker> bt_;

	bool freezed_;
	bool hasShown_;

	bool hScrollCellMove_;
	void updateHorizontalSliderMaximum();
};

#endif // ORDER_LIST_EDITOR_HPP
