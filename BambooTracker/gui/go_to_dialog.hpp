#ifndef GO_TO_DIALOG_HPP
#define GO_TO_DIALOG_HPP

#include <memory>
#include <QDialog>
#include "bamboo_tracker.hpp"

namespace Ui {
	class GoToDialog;
}

class GoToDialog : public QDialog
{
	Q_OBJECT

public:
	GoToDialog(std::weak_ptr<BambooTracker> bt, QWidget *parent = nullptr);
	~GoToDialog() override;

	int getOrder() const;
	int getStep() const;
	int getTrack() const;

private slots:
	void on_orderSpinBox_valueChanged(int arg1);

private:
	Ui::GoToDialog *ui;

	std::weak_ptr<BambooTracker> bt_;
	int song_;
};

#endif // GO_TO_DIALOG_HPP
