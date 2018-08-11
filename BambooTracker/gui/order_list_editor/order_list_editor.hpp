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
	explicit OrderListEditor(QWidget *parent = 0);
	~OrderListEditor();

	void setCore(std::shared_ptr<BambooTracker> core);

	void changeEditable();
	void insertNewRow(int prevRowNum);

signals:
	void currentTrackChanged(int num);

public slots:
	void setCurrentTrack(int num);

private:
	Ui::OrderListEditor *ui;
};

#endif // ORDER_LIST_EDITOR_HPP
