#ifndef ORDER_LIST_EDITOR_HPP
#define ORDER_LIST_EDITOR_HPP

#include <QFrame>
#include <QUndoStack>
#include <QEvent>
#include <memory>
#include "bamboo_tracker.hpp"

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

	void changeEditable();

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

public slots:
	void setCurrentTrack(int num);
	void setCurrentOrder(int num, int max);
	void onSongLoaded();

	void onPastePressed();
	/// 0: None
	/// 1: All
	void onSelectPressed(int type);

protected:
	bool eventFilter(QObject *watched, QEvent *event) override;

private:
	Ui::OrderListEditor *ui;
	std::shared_ptr<BambooTracker> bt_;
};

#endif // ORDER_LIST_EDITOR_HPP
