#ifndef CONFIGURATION_DIALOG_HPP
#define CONFIGURATION_DIALOG_HPP

#include <memory>
#include <unordered_map>
#include <vector>
#include <string>
#include <QDialog>
#include <QString>
#include <QKeySequenceEdit>
#include "configuration.hpp"
#include "color_palette.hpp"
#include "enum_hash.hpp"
#include "misc.hpp"

namespace Ui {
	class ConfigurationDialog;
}

class AudioStream;

class ConfigurationDialog : public QDialog
{
	Q_OBJECT

public:
	ConfigurationDialog(std::weak_ptr<Configuration> config, std::weak_ptr<ColorPalette> palette,
						std::weak_ptr<const AudioStream> stream, QWidget *parent = nullptr);
	~ConfigurationDialog() override;

signals:
	void applyPressed();

private slots:
	void on_ConfigurationDialog_accepted();

private:
	Ui::ConfigurationDialog *ui;
	std::weak_ptr<Configuration> config_;
	std::weak_ptr<ColorPalette> palette_;
	std::weak_ptr<const AudioStream> stream_;

	inline Qt::CheckState toCheckState(bool enabled)
	{
		return enabled ? Qt::Checked : Qt::Unchecked;
	}

	inline bool fromCheckState(Qt::CheckState state)
	{
		return (state == Qt::Checked) ? true : false;
	}

	std::unordered_map<JamKey, QKeySequenceEdit*> customLayoutKeysMap_;

	/***** General *****/
private slots:
	void on_generalSettingsListWidget_itemSelectionChanged();

	/***** Mixer *****/
private slots:
	void on_mixerResetPushButton_clicked();

	/***** Sound *****/
private slots:
	void on_midiInputChoiceButton_clicked();
	void on_soundAPIComboBox_currentIndexChanged(const QString &arg1);

	/***** Formats *****/
private:
	std::vector<FMEnvelopeText> fmEnvelopeTexts_;

private slots:
	void on_addEnvelopeSetPushButton_clicked();
	void on_removeEnvelopeSetpushButton_clicked();
	void on_editEnvelopeSetPushButton_clicked();
	void on_envelopeSetNameLineEdit_textChanged(const QString &arg1);
	void on_envelopeTypeListWidget_currentRowChanged(int currentRow);
	void updateEnvelopeSetUi();

	/***** Keys *****/
private slots:
	void on_keyboardTypeComboBox_currentIndexChanged(int index);
	void on_customLayoutResetButton_clicked();

private:
	std::unordered_map<Configuration::ShortcutAction, QKeySequenceEdit*> shortcutsMap_;

	void addShortcutItem(QString action, std::string shortcut);
	std::string getShortcutString(int row) const;

	/***** Appearance *****/
private slots:
	void on_colorEditPushButton_clicked();
	void on_colorLoadPushButton_clicked();
	void on_colorSavePushButton_clicked();

private:
	void updateColorTree();
};

#endif // CONFIGURATION_DIALOG_HPP
