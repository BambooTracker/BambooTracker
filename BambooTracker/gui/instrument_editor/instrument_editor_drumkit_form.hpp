#ifndef INSTRUMENT_EDITOR_DRUMKIT_FORM_HPP
#define INSTRUMENT_EDITOR_DRUMKIT_FORM_HPP

#include <memory>
#include <QWidget>
#include <QKeyEvent>
#include <QTreeWidgetItem>
#include "bamboo_tracker.hpp"
#include "configuration.hpp"
#include "jam_manager.hpp"
#include "gui/color_palette.hpp"

namespace Ui {
	class InstrumentEditorDrumkitForm;
}

class InstrumentEditorDrumkitForm : public QWidget
{
	Q_OBJECT

public:
	explicit InstrumentEditorDrumkitForm(int num, QWidget *parent = nullptr);
	~InstrumentEditorDrumkitForm() override;
	int getInstrumentNumber() const;
	void setCore(std::weak_ptr<BambooTracker> core);
	void setConfiguration(std::weak_ptr<Configuration> config);
	void setColorPalette(std::shared_ptr<ColorPalette> palette);
signals:
	void jamKeyOnEvent(JamKey key);
	void jamKeyOffEvent(JamKey key);
	void modified();

protected:
	void showEvent(QShowEvent*) override;
	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;

private:
	Ui::InstrumentEditorDrumkitForm *ui;
	int instNum_;
	bool isIgnoreEvent_;
	bool hasShown_;

	std::weak_ptr<BambooTracker> bt_;
	std::weak_ptr<Configuration> config_;
	std::shared_ptr<ColorPalette> palette_;

	void updateInstrumentParameters();

private slots:
	void on_keyTreeWidget_currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*);
	void on_pitshSpinBox_valueChanged(int arg1);

	//========== Sample ==========//
signals:
	void sampleNumberChanged();
	void sampleParameterChanged(int sampNum, int fromInstNum);
	void sampleAssignRequested();
	void sampleMemoryChanged();

public slots:
	void onSampleNumberChanged();
	void onSampleParameterChanged(int sampNum);
	void onSampleMemoryUpdated();

private slots:
	void on_sampleGroupBox_clicked(bool checked);

private:
	void setInstrumentSampleParameters(int key);
};

#endif // INSTRUMENT_EDITOR_DRUMKIT_FORM_HPP
