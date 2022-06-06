/*
 * Copyright (C) 2018-2022 Rerrah
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

#include <QObject>
#include <QSharedPointer>
#include <QHash>
#include <QSet>
#include "instrument/envelope_fm.hpp"
#include "bamboo_tracker_defs.hpp"
#include "utils.hpp"

class InstrumentEditor;
enum class InstrumentType;

// Hash function for QHash
constexpr uint qHash(const InstrumentType& type, uint seed)
{
	return qHash(static_cast<int>(type), seed);
}

/**
 * @brief The InstrumentEditorManager class
 */
class InstrumentEditorManager : public QObject
{
	Q_OBJECT

public:
	/**
	 * @brief Constructor.
	 */
	InstrumentEditorManager();

	/**
	 * @brief Update dialogs by changing configuration.
	 */
	void updateByConfiguration();

	/**
	 * @brief getDialog
	 * @param index Index of dialog list.
	 * @return Shared pointer of dialog when dialog list contains it, otherwise \c nullptr.
	 */
	const QSharedPointer<InstrumentEditor> getDialog(int index) const;

	/**
	 * @brief add Add instrument dialog to dialog list.
	 * @param index Index of instrument dialog in dialog list.
	 * @param Dialog shared pointer of dialog.
	 * @return \c true when dialog addition is success.
	 */
	bool add(int index, QSharedPointer<InstrumentEditor> dialog);

	/**
	 * @brief Swap dialogs in dialog list.
	 * @param index1 1st index of dialog.
	 * @param index2 2nd index of dialog.
	 */
	void swap(int index1, int index2);

	/**
	 * @brief Remove dialog from dialog list and dispose it.
	 *        If dialog is opened, it will be closed.
	 * @param index Index of dialog in dialog list.
	 * @return \c true when Dialog deletion is success.
	 */
	bool remove(int index);

	/**
	 * @brief showDialog
	 * @param index Index of dialog in dialog list.
	 */
	void showDialog(int index);

	/**
	 * @brief Close all managed dialogs.
	 */
	void closeAll();

	/**
	 * @brief Dispose all managed dialogs.
	 *        Opened dialogs are closed.
	 */
	void clearAll();

	/**
	 * @brief getActivatedDialogIndex
	 * @return Index of dialog in doalog list.
	 *         When no dialog is activated, returns -1.
	 */
	int getActivatedDialogIndex() const;

public slots:
	// Update FM parameters
	void onInstrumentFMEnvelopeParameterChanged(int envNum, int fromInstNum);
	void onInstrumentFMEnvelopeNumberChanged();
	void onInstrumentFMLFOParameterChanged(int lfoNum, int fromInstNum);
	void onInstrumentFMLFONumberChanged();
	void onInstrumentFMOperatorSequenceParameterChanged(FMEnvelopeParameter param, int opSeqNum, int fromInstNum);
	void onInstrumentFMOperatorSequenceNumberChanged();
	void onInstrumentFMArpeggioParameterChanged(int arpNum, int fromInstNum);
	void onInstrumentFMArpeggioNumberChanged();
	void onInstrumentFMPitchParameterChanged(int ptNum, int fromInstNum);
	void onInstrumentFMPitchNumberChanged();

	// Update SSG parameters
	void onInstrumentSSGWaveformParameterChanged(int wfNum, int fromInstNum);
	void onInstrumentSSGWaveformNumberChanged();
	void onInstrumentSSGToneNoiseParameterChanged(int tnNum, int fromInstNum);
	void onInstrumentSSGToneNoiseNumberChanged();
	void onInstrumentSSGEnvelopeParameterChanged(int envNum, int fromInstNum);
	void onInstrumentSSGEnvelopeNumberChanged();
	void onInstrumentSSGArpeggioParameterChanged(int arpNum, int fromInstNum);
	void onInstrumentSSGArpeggioNumberChanged();
	void onInstrumentSSGPitchParameterChanged(int ptNum, int fromInstNum);
	void onInstrumentSSGPitchNumberChanged();

	// Update ADPCM/Drumkit parameters
	void onInstrumentADPCMSampleParameterChanged(int sampNum, int fromInstNum);
	void onInstrumentADPCMSampleNumberChanged();
	void onInstrumentADPCMSampleMemoryUpdated();
	void onInstrumentADPCMEnvelopeParameterChanged(int envNum, int fromInstNum);
	void onInstrumentADPCMEnvelopeNumberChanged();
	void onInstrumentADPCMArpeggioParameterChanged(int arpNum, int fromInstNum);
	void onInstrumentADPCMArpeggioNumberChanged();
	void onInstrumentADPCMPitchParameterChanged(int ptNum, int fromInstNum);
	void onInstrumentADPCMPitchNumberChanged();

private:
	// Size of dialog list
	enum { MAX_NUM_DIALOG = 128 };
	// Dialog list
	QSharedPointer<InstrumentEditor> dialogs_[MAX_NUM_DIALOG];
	// Map of instrument type and dialog index list
	QHash<InstrumentType, QSet<int>> typeIndices_;

	// Assert that given index is valid for dialog list.
	static constexpr bool assertIndex(int index) { return utils::isInRange<int>(index, 0, MAX_NUM_DIALOG); }

	// Get indices of dialog which is neccessary to update parameters.
	inline QSet<int> getIndicesForParameterUpdating(InstrumentType type, int skippedIdx = -1)
	{
		auto idcs = typeIndices_[type];
		if (assertIndex(skippedIdx)) idcs.remove(skippedIdx);
		return idcs;
	}
};
