#ifndef ORDER_LIST_EDITOR_HPP
#define ORDER_LIST_EDITOR_HPP

#include <QFrame>
#include <QUndoStack>
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
	~OrderListEditor();

	void setCore(std::shared_ptr<BambooTracker> core);
	void setCommandStack(std::weak_ptr<QUndoStack> stack);

	void changeEditable();

signals:
	void currentTrackChanged(int num);
	void currentOrderChanged(int num);

	void orderEdited();

public slots:
	void setCurrentTrack(int num);
	void setCurrentOrder(int num, int max);
	void onSongLoaded();

private:
	Ui::OrderListEditor *ui;
	std::shared_ptr<BambooTracker> bt_;
};

#endif // ORDER_LIST_EDITOR_HPP
