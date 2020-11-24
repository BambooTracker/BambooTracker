/*
 * Copyright (C) 2018-2020 Rerrah
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

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
	std::weak_ptr<ColorPalette> refPalette_;
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
	void on_audioApiComboBox_currentIndexChanged(const QString &arg1);
	void on_midiApiComboBox_currentIndexChanged(const QString &arg1);
	void onMidiApiChanged(const QString &arg1, bool hasInitialized = true);

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
	void updateColorTreeFrom(const ColorPalette* const palette);
	void setPaletteFromColorTree(ColorPalette* const palette);
};

#endif // CONFIGURATION_DIALOG_HPP
