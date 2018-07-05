#ifndef INSTRUMENTEDITORFMFORM_HPP
#define INSTRUMENTEDITORFMFORM_HPP

#include <QWidget>

namespace Ui {
	class InstrumentEditorFMForm;
}

class InstrumentEditorFMForm : public QWidget
{
	Q_OBJECT

public:
	explicit InstrumentEditorFMForm(QWidget *parent = 0);
	~InstrumentEditorFMForm();

private:
	Ui::InstrumentEditorFMForm *ui;
};

#endif // INSTRUMENTEDITORFMFORM_HPP
