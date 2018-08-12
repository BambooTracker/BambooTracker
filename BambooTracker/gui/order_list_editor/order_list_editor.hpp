#ifndef ORDER_LIST_EDITOR_HPP
#define ORDER_LIST_EDITOR_HPP

#include <QFrame>
#include <QEvent>
#include <QMouseEvent>
#include <memory>
#include "bamboo_tracker.hpp"
#include "module.hpp"

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

	void changeEditable();

signals:
	void currentTrackChanged(int num);
	void currentOrderChanged(int num);

public slots:
	void setCurrentTrack(int num);
	void setCurrentOrder(int num);

private:
	Ui::OrderListEditor *ui;
};

#endif // ORDER_LIST_EDITOR_HPP
