/*
 * Copyright (C) 2018-2021 Rerrah
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

#ifndef MODULE_PROPERTIES_DIALOG_HPP
#define MODULE_PROPERTIES_DIALOG_HPP

#include <memory>
#include <unordered_map>
#include <QDialog>
#include <QString>
#include <QTreeWidgetItem>
#include "bamboo_tracker.hpp"
#include "enum_hash.hpp"

namespace Ui {
	class ModulePropertiesDialog;
}

class ModulePropertiesDialog : public QDialog
{
	Q_OBJECT

public:
	ModulePropertiesDialog(std::weak_ptr<BambooTracker> core, double configFmMixer,
						   double configSsgMixer, QWidget *parent = nullptr);
	~ModulePropertiesDialog() override;

public slots:
	void onAccepted();

private slots:
	void on_upToolButton_clicked();
	void on_downToolButton_clicked();
	void on_removePushButton_clicked();
	void on_insertPushButton_clicked();
	void on_songTreeWidget_itemSelectionChanged();
	void on_mixerTypeComboBox_currentIndexChanged(int index);
	void on_customMixerSetPushButton_clicked();
	void on_updateButton_clicked();

private:
	Ui::ModulePropertiesDialog *ui;
	std::weak_ptr<BambooTracker> bt_;

	double fmMixer_, ssgMixer_;
	double configFmMixer_, configSsgMixer_;

	static const std::unordered_map<SongType, QString> SONG_TYPE_TEXT_;

	void insertSong(int row, QString title, SongType type, int prevNum = -1);
	void checkButtonsEnabled();
	void swapset(int aboveRow, int belowRow);

	void setCustomMixerLevels(double fm, double ssg);
};

#endif // MODULE_PROPERTIES_DIALOG_HPP
