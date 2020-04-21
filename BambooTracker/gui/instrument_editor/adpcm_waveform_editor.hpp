#ifndef ADPCM_WAVEFORM_EDITOR_HPP
#define ADPCM_WAVEFORM_EDITOR_HPP

#include <memory>
#include <QWidget>
#include <QEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QPixmap>
#include <QString>
#include "bamboo_tracker.hpp"
#include "configuration.hpp"
#include "gui/color_palette.hpp"

namespace Ui {
	class ADPCMWaveformEditor;
}

class ADPCMWaveformEditor : public QWidget
{
	Q_OBJECT

public:
	explicit ADPCMWaveformEditor(QWidget *parent = nullptr);
	~ADPCMWaveformEditor() override;
	void setNumber(int n);
	void setCore(std::weak_ptr<BambooTracker> core);
	void setConfiguration(std::weak_ptr<Configuration> config);
	void setColorPalette(std::shared_ptr<ColorPalette> palette);

signals:
	void modified();
	void waveformNumberChanged();
	void waveformParameterChanged(int wfNum, int fromInstNum);
	void waveformAssignRequested();

public slots:
	void onWaveformNumberChanged();
	void onWaveformParameterChanged(int wfNum);
	void onWaveformSampleMemoryUpdated();

protected:
	bool eventFilter(QObject* obj, QEvent* ev) override;
	void dragEnterEvent(QDragEnterEvent* event) override;
	void dropEvent(QDropEvent* event) override;

private:
	Ui::ADPCMWaveformEditor *ui;
	int instNum_;
	bool isIgnoreEvent_;

	std::weak_ptr<BambooTracker> bt_;
	std::weak_ptr<Configuration> config_;
	std::shared_ptr<ColorPalette> palette_;

	std::unique_ptr<QPixmap> wavMemPixmap_;
	std::unique_ptr<QPixmap> wavViewPixmap_;

	void setInstrumentWaveformParameters();
	void importSampleFrom(const QString file);
	void updateSampleMemoryBar();
	void updateSampleView();

private slots:
	void on_waveNumSpinBox_valueChanged(int arg1);
	void on_rootRateSpinBox_valueChanged(int arg1);
	void on_waveRepeatCheckBox_toggled(bool checked);
	void on_waveImportPushButton_clicked();
	void on_waveClearPushButton_clicked();
};

#endif // ADPCM_WAVEFORM_EDITOR_HPP
