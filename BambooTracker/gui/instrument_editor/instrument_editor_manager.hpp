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
#include <QPointer>
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
	 * @brief Destructor.
	 */
	~InstrumentEditorManager();

	/**
	 * @brief Update editors by changing configuration.
	 */
	void updateByConfiguration();

	/**
	 * @brief add Add instrument editor to editor list.
	 * @param index Index of instrument editor in editor list.
	 * @param editor Shared pointer of editor.
	 * @return \c true when editor addition is success.
	 */
	bool add(int index, InstrumentEditor* editor);

	/**
	 * @brief Swap editors in editor list.
	 * @param index1 1st index of editor.
	 * @param index2 2nd index of editor.
	 */
	void swap(int index1, int index2);

	/**
	 * @brief Remove editor from editor list and dispose it.
	 *        If editor is opened, it will be closed.
	 * @param index Index of editor in editor list.
	 * @return \c true when editor deletion is success.
	 */
	bool remove(int index);

	/**
	 * @brief showEditor
	 * @param index Index of editor in editor list.
	 */
	void showEditor(int index);

	/**
	 * @brief Dispose all managed editors.
	 *        Opened editors are closed.
	 */
	void removeAll();

	/**
	 * @brief hasShownEditor
	 * @param index Index of editor in editor list.
	 * @return \c true when editor list registers editor.
	 */
	bool hasShownEditor(int index) const;

	/**
	 * @brief getActivatedEditorIndex
	 * @return Index of editor in doalog list.
	 *         When no editor is activated, returns -1.
	 */
	int getActivatedEditorIndex() const;

public slots:
	// Update windows title
	void onInstrumentNameChanged(int instNum);

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
	// Size of editor list
	enum { MAX_NUM_EDITOR = 128 };
	// Editor list
	QPointer<InstrumentEditor> editors_[MAX_NUM_EDITOR];
	// Map of instrument type and editor index list
	QHash<InstrumentType, QSet<int>> typeIndices_;

	// Assert that given index is valid for editor list.
	static constexpr bool assertIndex(int index) { return utils::isInRange<int>(index, 0, MAX_NUM_EDITOR); }

	// Get indices of editor which is neccessary to update parameters.
	inline QSet<int> getIndicesForParameterUpdating(InstrumentType type, int skippedIdx = -1)
	{
		auto idcs = typeIndices_[type];
		if (assertIndex(skippedIdx)) idcs.remove(skippedIdx);
		return idcs;
	}
};
