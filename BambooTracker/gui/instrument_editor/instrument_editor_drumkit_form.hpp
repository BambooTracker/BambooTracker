#ifndef INSTRUMENT_EDITOR_DRUMKIT_FORM_HPP
#define INSTRUMENT_EDITOR_DRUMKIT_FORM_HPP

#include <QWidget>

namespace Ui {
	class InstrumentEditorDrumkitForm;
}

class InstrumentEditorDrumkitForm : public QWidget
{
	Q_OBJECT

public:
	explicit InstrumentEditorDrumkitForm(QWidget *parent = nullptr);
	~InstrumentEditorDrumkitForm();

private:
	Ui::InstrumentEditorDrumkitForm *ui;
};

#endif // INSTRUMENT_EDITOR_DRUMKIT_FORM_HPP
