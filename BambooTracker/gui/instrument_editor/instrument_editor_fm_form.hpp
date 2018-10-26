#ifndef INSTRUMENT_EDITOR_FM_FORM_HPP
#define INSTRUMENT_EDITOR_FM_FORM_HPP

#include <QWidget>
#include <QKeyEvent>
#include <memory>
#include "bamboo_tracker.hpp"
#include "instrument.hpp"
#include "gui/instrument_editor/visualized_instrument_macro_editor.hpp"

namespace Ui {
	class InstrumentEditorFMForm;
}

class InstrumentEditorFMForm : public QWidget
{
	Q_OBJECT

public:
	InstrumentEditorFMForm(int num, QWidget *parent = nullptr);
	~InstrumentEditorFMForm() override;
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
	Ui::InstrumentEditorFMForm *ui;
	int instNum_;
	bool isIgnoreEvent_;

	std::weak_ptr<BambooTracker> bt_;

	ReleaseType convertReleaseTypeForData(VisualizedInstrumentMacroEditor::ReleaseType type);
	VisualizedInstrumentMacroEditor::ReleaseType convertReleaseTypeForUI(ReleaseType type);

	void updateInstrumentParameters();

	//========== Envelope ==========//
signals:
	void envelopeNumberChanged();
	void envelopeParameterChanged(int envNum, int fromInstNum);

public slots:
	void onEnvelopeParameterChanged(int envNum);
	void onEnvelopeNumberChanged();

private:
	void setInstrumentEnvelopeParameters();
	void setInstrumentEnvelopeParameters(QString data);

	QString toEnvelopeString() const;

private slots:
	void on_envNumSpinBox_valueChanged(int arg1);
	void on_envGroupBox_customContextMenuRequested(const QPoint &pos);

	//========== LFO ==========//
signals:
	void lfoNumberChanged();
	void lfoParameterChanged(int lfoNum, int fromInstNum);

public slots:
	void onLFOParameterChanged(int lfoNum);
	void onLFONumberChanged();

private:
	void setInstrumentLFOParameters();
	void setInstrumentLFOParameters(QString data);

	QString toLFOString() const;

private slots:
	void on_lfoGroupBox_customContextMenuRequested(const QPoint &pos);
	void on_lfoNumSpinBox_valueChanged(int arg1);
	void on_lfoGroupBox_toggled(bool arg1);

	//========== OperatorSequence ==========//
public:
	FMEnvelopeParameter getOperatorSequenceParameter() const;

signals:
	void operatorSequenceNumberChanged();
	void operatorSequenceParameterChanged(FMEnvelopeParameter param, int opSeqNum, int fromInstNum);

public slots:
	void onOperatorSequenceNumberChanged();
	void onOperatorSequenceParameterChanged(FMEnvelopeParameter param, int opSeqNum);

private:
	void setInstrumentOperatorSequenceParameters();
	void setOperatorSequenceEditor();

private slots:
	void onOperatorSequenceTypeChanged(int type);
	void on_opSeqEditGroupBox_toggled(bool arg1);
	void on_opSeqNumSpinBox_valueChanged(int arg1);

	//========== Arpeggio ==========//
signals:
	void arpeggioNumberChanged();
	void arpeggioParameterChanged(int arpNum, int fromInstNum);

public slots:
	void onArpeggioNumberChanged();
	void onArpeggioParameterChanged(int arpNum);

private:
	void setInstrumentArpeggioParameters();

private slots:
	void onArpeggioTypeChanged(int index);
	void on_arpEditGroupBox_toggled(bool arg1);
	void on_arpNumSpinBox_valueChanged(int arg1);

	//========== Pitch ==========//
signals:
	void pitchNumberChanged();
	void pitchParameterChanged(int ptNum, int fromInstNum);

public slots:
	void onPitchNumberChanged();
	void onPitchParameterChanged(int arpNum);

private:
	void setInstrumentPitchParameters();

private slots:
	void onPitchTypeChanged(int index);
	void on_ptEditGroupBox_toggled(bool arg1);
	void on_ptNumSpinBox_valueChanged(int arg1);

	//========== Else ==========//
private slots:
	void on_envResetCheckBox_stateChanged(int arg1);
};

#endif // INSTRUMENT_EDITOR_FM_FORM_HPP
