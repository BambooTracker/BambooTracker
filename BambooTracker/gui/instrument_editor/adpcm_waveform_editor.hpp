#ifndef ADPCM_WAVEFORM_EDITOR_HPP
#define ADPCM_WAVEFORM_EDITOR_HPP

#include <memory>
#include <cstdint>
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
	void setCore(std::weak_ptr<BambooTracker> core);
	void setConfiguration(std::weak_ptr<Configuration> config);
	void setColorPalette(std::shared_ptr<ColorPalette> palette);

	int getWaveformNumber() const;

	void setInstrumentWaveformParameters(int wfNum, bool repeatable, int rKeyNum, int rDeltaN,
										 size_t start, size_t stop, std::vector<uint8_t> sample);

signals:
	void modified();
	void waveformNumberChanged(int n);	// NEED Direct connection
	void waveformParameterChanged(int wfNum);
	void waveformAssignRequested();
	void waveformMemoryChanged();

public slots:
	void onWaveformNumberChanged();
	void onWaveformSampleMemoryUpdated(size_t start, size_t stop);

protected:
	bool eventFilter(QObject* obj, QEvent* ev) override;
	void dragEnterEvent(QDragEnterEvent* event) override;
	void dropEvent(QDropEvent* event) override;

private:
	Ui::ADPCMWaveformEditor *ui;
	bool isIgnoreEvent_;

	std::weak_ptr<BambooTracker> bt_;
	std::weak_ptr<Configuration> config_;
	std::shared_ptr<ColorPalette> palette_;

	std::unique_ptr<QPixmap> wavMemPixmap_;
	std::unique_ptr<QPixmap> wavViewPixmap_;

	size_t addrStart_, addrStop_;
	std::vector<uint8_t> sample_;

	void importSampleFrom(const QString file);
	void updateSampleMemoryBar();
	void updateSampleView();
	void updateUsersView();

private slots:
	void on_waveNumSpinBox_valueChanged(int arg1);
	void on_rootRateSpinBox_valueChanged(int arg1);
	void on_waveRepeatCheckBox_toggled(bool checked);
	void on_waveImportPushButton_clicked();
	void on_waveClearPushButton_clicked();
};

#endif // ADPCM_WAVEFORM_EDITOR_HPP
