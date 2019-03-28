#ifndef CONFIGURATION_DIALOG_HPP
#define CONFIGURATION_DIALOG_HPP

#include <QDialog>
#include <memory>
#include <vector>
#include "configuration.hpp"
#include "misc.hpp"

namespace Ui {
	class ConfigurationDialog;
}

class ConfigurationDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ConfigurationDialog(std::weak_ptr<Configuration> config, QWidget *parent = nullptr);
	~ConfigurationDialog() override;

signals:
	void applyPressed();

private slots:
	void on_ConfigurationDialog_accepted();

private:
	Ui::ConfigurationDialog *ui;
	std::weak_ptr<Configuration> config_;

	inline Qt::CheckState toCheckState(bool enabled)
	{
		return enabled ? Qt::Checked : Qt::Unchecked;
	}

	inline bool fromCheckState(Qt::CheckState state)
	{
		return (state == Qt::Checked) ? true : false;
	}

	/***** General *****/
private slots:
	void on_generalSettingsListWidget_itemSelectionChanged();

	/***** Mixer *****/
private slots:
	void on_mixerResetPushButton_clicked();

	/***** MIDI *****/
private slots:
	void on_midiInputChoiceButton_clicked();

	/***** Input *****/
private:
	std::vector<FMEnvelopeText> fmEnvelopeTexts_;

	void updateEnvelopeSetUi();

private slots:
	void on_addEnvelopeSetPushButton_clicked();
	void on_removeEnvelopeSetpushButton_clicked();
	void on_editEnvelopeSetPushButton_clicked();
	void on_envelopeSetNameLineEdit_textChanged(const QString &arg1);
	void on_envelopeTypeListWidget_currentRowChanged(int currentRow);
};

#endif // CONFIGURATION_DIALOG_HPP
