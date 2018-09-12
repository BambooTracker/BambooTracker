#ifndef INSTRUMENT_EDITOR_SSG_FORM_HPP
#define INSTRUMENT_EDITOR_SSG_FORM_HPP

#include <QWidget>
#include <QKeyEvent>
#include <memory>
#include "bamboo_tracker.hpp"
#include "instrument.hpp"

namespace Ui {
	class InstrumentEditorSSGForm;
}

class InstrumentEditorSSGForm : public QWidget
{
	Q_OBJECT

public:
	InstrumentEditorSSGForm(int num, QWidget *parent = nullptr);
	~InstrumentEditorSSGForm() override;
	int getInstrumentNumber() const;
	void setCore(std::weak_ptr<BambooTracker> core);

signals:
	void jamKeyOnEvent(QKeyEvent* event);
	void jamKeyOffEvent(QKeyEvent* event);
	void octaveChanged(bool upFlag);
	void modified();

protected:
	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;

private:
	Ui::InstrumentEditorSSGForm *ui;
	int instNum_;
	bool isIgnoreEvent_;

	std::weak_ptr<BambooTracker> bt_;

	void updateInstrumentParameters();

	//========== Wave form ==========//

	//========== Else ==========//
private slots:
	void on_gateCountSpinBox_valueChanged(int arg1);
};

#endif // INSTRUMENT_EDITOR_SSG_FORM_HPP
