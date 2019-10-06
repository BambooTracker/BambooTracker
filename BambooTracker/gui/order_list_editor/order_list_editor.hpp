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
	void setConfiguration(std::weak_ptr<Configuration> config);
	void setColorPallete(std::shared_ptr<ColorPalette> palette);

	void changeEditable();
	void updatePositionByOrderUpdate();

	void copySelectedCells();
	void deleteOrder();
	void insertOrderBelow();

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

protected:
	bool eventFilter(QObject *watched, QEvent *event) override;

private:
	Ui::OrderListEditor *ui;
	std::shared_ptr<BambooTracker> bt_;
};

#endif // ORDER_LIST_EDITOR_HPP
