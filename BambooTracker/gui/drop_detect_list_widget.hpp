#ifndef DROP_DETECT_LIST_WIDGET_HPP
#define DROP_DETECT_LIST_WIDGET_HPP

#include <QListWidget>

class DropDetectListWidget : public QListWidget
{
	Q_OBJECT

public:
	explicit DropDetectListWidget(QWidget* parent = nullptr);

signals:
	void itemDroppedAtItemIndex(int dropped, int target);

protected:
	void dropEvent(QDropEvent* event) override;
};

#endif // DROP_DETECT_LIST_WIDGET_HPP
