/*
 * Copyright (C) 2022 Rerrah
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

#include <memory>
#include <QDialog>
#include "bamboo_tracker.hpp"
#include "configuration.hpp"
#include "jamming.hpp"
#include "gui/color_palette.hpp"
#include "bamboo_tracker_defs.hpp"

enum class InstrumentType: int;

/**
 * @brief The InstrumentEditor class
 */
class InstrumentEditor : public QDialog
{
	Q_OBJECT

public:
	/**
	 * @brief Return sound source of instrument related to this dialog.
	 * @return Sound source of instrument.
	 */
	virtual SoundSource getSoundSource() const = 0;

	/**
	 * @brief Return instrument type of instrument related to this dialog.
	 * @return Instrument type of instrument.
	 */
	virtual InstrumentType getInstrumentType() const = 0;

	/**
	 * @brief setInstrumentNumber
	 * @param num Instrument number.
	 */
	void setInstrumentNumber(int num) { instNum_ = num; }

	/**
	 * @brief getInstrumentNumber
	 * @return Instrument number.
	 */
	int getInstrumentNumber() const noexcept { return instNum_; }

	/**
	 * @brief setCore
	 * @param core Weak pointer to core.
	 */
	void setCore(std::weak_ptr<BambooTracker> core);

	/**
	 * @brief setConfiguration
	 * @param config Weak pointer to configuration.
	 */
	void setConfiguration(std::weak_ptr<Configuration> config);

	/**
	 * @brief setColorPalette
	 * @param palette
	 */
	void setColorPalette(std::shared_ptr<ColorPalette> palette);

	/**
	 * @brief updateByConfigurationChange
	 */
	virtual void updateByConfigurationChange() = 0;

signals:
	/**
	 * @brief Emitted on jamming key on event.
	 * @param key Jamkey
	 */
	void jamKeyOnEvent(JamKey key);

	/**
	 * @brief Emitted on jamming key off event.
	 * @param key Jamkey
	 */
	void jamKeyOffEvent(JamKey key);

	/**
	 * @brief Emitted when something in dialog is edited.
	 */
	void modified();

protected:
	// Instrument number
	int instNum_;

	// Pointer to core
	std::weak_ptr<BambooTracker> bt_;
	// Pointer to color palette
	std::shared_ptr<ColorPalette> palette_;
	// Pointer to configuration
	std::weak_ptr<Configuration> config_;

	// Constructor.
	explicit InstrumentEditor(int num, QWidget* parent = nullptr);

	// Instrument specific process called in settiing core / cofiguration / color palette.
	virtual void updateBySettingCore() = 0;
	virtual void updateBySettingConfiguration() = 0;
	virtual void updateBySettingColorPalette() = 0;
};
