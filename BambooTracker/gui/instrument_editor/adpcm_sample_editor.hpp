#ifndef ADPCM_SAMPLE_EDITOR_HPP
#define ADPCM_SAMPLEEDITOR_HPP

#include <memory>
#include <cstdint>
#include <QWidget>
#include <QEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QPixmap>
#include <QString>
#include <QPoint>
#include "bamboo_tracker.hpp"
#include "configuration.hpp"
#include "gui/color_palette.hpp"

namespace Ui {
	class ADPCMSampleEditor;
}

class ADPCMSampleEditor : public QWidget
{
	Q_OBJECT

public:
	explicit ADPCMSampleEditor(QWidget *parent = nullptr);
	~ADPCMSampleEditor() override;
	void setCore(std::weak_ptr<BambooTracker> core);
	void setConfiguration(std::weak_ptr<Configuration> config);
	void setColorPalette(std::shared_ptr<ColorPalette> palette);

	int getSampleNumber() const;

	void setInstrumentSampleParameters(int sampNum, bool repeatable, int rKeyNum, int rDeltaN,
									   size_t start, size_t stop, std::vector<uint8_t> sample);

signals:
	void modified();
	void sampleNumberChanged(int n);	// NEED Direct connection
	void sampleParameterChanged(int wfNum);
	void sampleAssignRequested();
	void sampleMemoryChanged();

public slots:
	void onSampleNumberChanged();
	void onSampleMemoryUpdated(size_t start, size_t stop);

protected:
	bool eventFilter(QObject* obj, QEvent* ev) override;
	void dragEnterEvent(QDragEnterEvent* event) override;
	void dropEvent(QDropEvent* event) override;

private:
	Ui::ADPCMSampleEditor *ui;
	bool isIgnoreEvent_;

	std::weak_ptr<BambooTracker> bt_;
	std::weak_ptr<Configuration> config_;
	std::shared_ptr<ColorPalette> palette_;

	std::unique_ptr<QPixmap> memPixmap_;
	std::unique_ptr<QPixmap> sampViewPixmap_;
	int zoom_, gridIntr_;
	QPoint cursorSamp_, prevPressedSamp_;

	size_t addrStart_, addrStop_;
	std::vector<int16_t> sample_;

	void importSampleFrom(const QString file);
	void updateSampleMemoryBar();
	void updateSampleView();
	void updateUsersView();

	void detectCursorSamplePosition(int cx, int cy);

	inline QString updateDetailView() const
	{
		return tr("(%1, %2), %3, x%4")
				.arg(cursorSamp_.x()).arg(cursorSamp_.y()).arg(sample_.size()).arg(zoom_ + 1);
	}

private slots:
	void on_sampleNumSpinBox_valueChanged(int arg1);
	void on_repeatCheckBox_toggled(bool checked);
	void on_rootRateSpinBox_valueChanged(int arg1);
	void on_action_Resize_triggered();
	void on_actionRe_verse_triggered();
	void on_actionZoom_In_triggered();
	void on_actionZoom_Out_triggered();
	void on_horizontalScrollBar_valueChanged(int value);
	void on_action_Import_triggered();
	void on_action_Clear_triggered();
	void on_action_Grid_View_triggered();
	void on_actionG_rid_Settings_triggered();
};

#endif // ADPCM_SAMPLE_EDITOR_HPP
