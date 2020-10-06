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

#pragma once

#include <QDialog>
#include <QVector>
#include <memory>
#include "configuration.hpp"

class AbstractBank;

namespace Ui {
	class InstrumentSelectionDialog;
}

class InstrumentSelectionDialog : public QDialog
{
	Q_OBJECT

public:
	InstrumentSelectionDialog(const AbstractBank &bank, const QString &text,
							  std::weak_ptr<Configuration> config, QWidget *parent = nullptr);
	~InstrumentSelectionDialog() override;

	QVector<size_t> currentInstrumentSelection() const;

public slots:
	void onJamKeyOnByMidi(int key);
	void onJamKeyOffByMidi(int key);

signals:
	void jamKeyOnEvent(size_t id, JamKey key);
	void jamKeyOnMidiEvent(size_t id, int key);
	void jamKeyOffEvent(JamKey key);
	void jamKeyOffMidiEvent(int key);

protected:
	bool eventFilter(QObject* watched, QEvent* event) override;

private:
	const AbstractBank &bank_;
	std::weak_ptr<Configuration> config_;
	std::unique_ptr<Ui::InstrumentSelectionDialog> ui_;

	void setupContents();

private slots:
	void on_searchLineEdit_textChanged(const QString &search);
};
