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
	void updatePositionByPositionJump(bool trackChanged = false);

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

	void setHorizontalScrollMode(bool cellBased, bool refresh = true);
	void setVisibleTracks(std::vector<int> tracks);

signals:
	void currentTrackChanged(int idx);
	void currentOrderChanged(int num);

	void orderEdited();

	void focusIn();
	void focusOut();
	void selected(bool isSelected);

public slots:
	void onPatternEditorCurrentTrackChanged(int idx);
	void onPatternEditorCurrentOrderChanged(int num, int max);
	void onSongLoaded();
	void onShortcutUpdated();

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
	void onGoOrderRequested(bool toNext);

	void onOrderDataGlobalChanged();

protected:
	bool eventFilter(QObject* watched, QEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;

private:
	Ui::OrderListEditor *ui;
	std::shared_ptr<BambooTracker> bt_;

	bool freezed_;
	bool songLoaded_;

	bool hScrollCellMove_;
	void updateHorizontalSliderMaximum();
	void updateMaximumWidth();
};

#endif // ORDER_LIST_EDITOR_HPP
