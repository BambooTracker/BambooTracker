#ifndef INSTRUMENTEDITORFMFORM_HPP
#define INSTRUMENTEDITORFMFORM_HPP

#include <QWidget>
#include <memory>
#include "bamboo_tracker.hpp"
#include "instrument.hpp"

namespace Ui {
	class InstrumentEditorFMForm;
}

class InstrumentEditorFMForm : public QWidget
{
	Q_OBJECT

public:
	InstrumentEditorFMForm(int num, QWidget *parent = 0);
	~InstrumentEditorFMForm();
	void setCore(std::shared_ptr<BambooTracker> core);

private:
	Ui::InstrumentEditorFMForm *ui;
	int instNum_;
	bool isValidEmit;

	std::shared_ptr<BambooTracker> bt_;

	void setInstrumentParameters();
};

#endif // INSTRUMENTEDITORFMFORM_HPP
