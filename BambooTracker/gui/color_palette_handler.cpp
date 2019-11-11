#include "color_palette_handler.hpp"
#include <QSettings>
#include <QColor>

// config path (*nix): ~/.config/<organization>/<application>.ini
const QString ColorPaletteHandler::organization = "BambooTracker";
const QString ColorPaletteHandler::application = "BambooTracker";

ColorPaletteHandler::ColorPaletteHandler() {}

bool ColorPaletteHandler::savePalette(std::weak_ptr<ColorPalette> palette)
{
	try {
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, ColorPaletteHandler::organization, ColorPaletteHandler::application);
		save(settings, palette);
		return true;
	} catch (...) {
		return false;
	}
}

bool ColorPaletteHandler::savePalette(QString file, std::weak_ptr<ColorPalette> palette)
{
	try {
		QSettings settings(file, QSettings::IniFormat);
		save(settings, palette);
		return true;
	} catch (...) {
		return false;
	}
}

void ColorPaletteHandler::save(QSettings& settings, std::weak_ptr<ColorPalette> palette)
{
	std::shared_ptr<ColorPalette> paletteLocked = palette.lock();

	settings.beginGroup("PatternEditor");
	settings.setValue("defaultStepText", paletteLocked->ptnDefTextColor.name(QColor::HexArgb));
	settings.setValue("defaultStepBackground", paletteLocked->ptnDefStepColor.name(QColor::HexArgb));
	settings.setValue("highlightedStep1Background", paletteLocked->ptnHl1StepColor.name(QColor::HexArgb));
	settings.setValue("highlightedStep2Background", paletteLocked->ptnHl2StepColor.name(QColor::HexArgb));
	settings.setValue("currentStepText", paletteLocked->ptnCurTextColor.name(QColor::HexArgb));
	settings.setValue("currentStepBackground", paletteLocked->ptnCurStepColor.name(QColor::HexArgb));
	settings.setValue("currentEditingStepBackground", paletteLocked->ptnCurEditStepColor.name(QColor::HexArgb));
	settings.setValue("currentCellBackground", paletteLocked->ptnCurCellColor.name(QColor::HexArgb));
	settings.setValue("currentPlayingStepBackground", paletteLocked->ptnPlayStepColor.name(QColor::HexArgb));
	settings.setValue("selectionBackground", paletteLocked->ptnSelCellColor.name(QColor::HexArgb));
	settings.setValue("hoveredCellBackground", paletteLocked->ptnHovCellColor.name(QColor::HexArgb));
	settings.setValue("defaultStepNumber", paletteLocked->ptnDefStepNumColor.name(QColor::HexArgb));
	settings.setValue("highlightedStep1Number", paletteLocked->ptnHl1StepNumColor.name(QColor::HexArgb));
	settings.setValue("highlightedStep2Number", paletteLocked->ptnHl2StepNumColor.name(QColor::HexArgb));
	settings.setValue("noteText", paletteLocked->ptnNoteColor.name(QColor::HexArgb));
	settings.setValue("instrumentText", paletteLocked->ptnInstColor.name(QColor::HexArgb));
	settings.setValue("volumeText", paletteLocked->ptnVolColor.name(QColor::HexArgb));
	settings.setValue("effectText", paletteLocked->ptnEffColor.name(QColor::HexArgb));
	settings.setValue("errorText", paletteLocked->ptnErrorColor.name(QColor::HexArgb));
	settings.setValue("headerText", paletteLocked->ptnHeaderTextColor.name(QColor::HexArgb));
	settings.setValue("headerBackground", paletteLocked->ptnHeaderRowColor.name(QColor::HexArgb));
	settings.setValue("mask", paletteLocked->ptnMaskColor.name(QColor::HexArgb));
	settings.setValue("border", paletteLocked->ptnBorderColor.name(QColor::HexArgb));
	settings.setValue("mute", paletteLocked->ptnMuteColor.name(QColor::HexArgb));
	settings.setValue("unmute", paletteLocked->ptnUnmuteColor.name(QColor::HexArgb));
	settings.setValue("background", paletteLocked->ptnBackColor.name(QColor::HexArgb));
	settings.endGroup();

	settings.beginGroup("OrderList");
	settings.setValue("defaultRowText", paletteLocked->odrDefTextColor.name(QColor::HexArgb));
	settings.setValue("defaultRowBackground", paletteLocked->odrDefRowColor.name(QColor::HexArgb));
	settings.setValue("currentRowText", paletteLocked->odrCurTextColor.name(QColor::HexArgb));
	settings.setValue("currentRowBackground", paletteLocked->odrCurRowColor.name(QColor::HexArgb));
	settings.setValue("currentEditingRowBackground", paletteLocked->odrCurEditRowColor.name(QColor::HexArgb));
	settings.setValue("currentCellBackground", paletteLocked->odrCurCellColor.name(QColor::HexArgb));
	settings.setValue("currentPlayingRowBackground", paletteLocked->odrPlayRowColor.name(QColor::HexArgb));
	settings.setValue("selectionBackground", paletteLocked->odrSelCellColor.name(QColor::HexArgb));
	settings.setValue("hoveredCellBackground", paletteLocked->odrHovCellColor.name(QColor::HexArgb));
	settings.setValue("rowNumber", paletteLocked->odrRowNumColor.name(QColor::HexArgb));
	settings.setValue("headerText", paletteLocked->odrHeaderTextColor.name(QColor::HexArgb));
	settings.setValue("headerBackground", paletteLocked->odrHeaderRowColor.name(QColor::HexArgb));
	settings.setValue("border", paletteLocked->odrBorderColor.name(QColor::HexArgb));
	settings.setValue("background", paletteLocked->odrBackColor.name(QColor::HexArgb));
	settings.endGroup();

	settings.beginGroup("InstrumentList");
	settings.setValue("defaultText", paletteLocked->ilistTextColor.name(QColor::HexArgb));
	settings.setValue("background", paletteLocked->ilistBackColor.name(QColor::HexArgb));
	settings.setValue("selectedBackground", paletteLocked->ilistSelBackColor.name(QColor::HexArgb));
	settings.setValue("hoveredBackground", paletteLocked->ilistHovBackColor.name(QColor::HexArgb));
	settings.setValue("selectedHoveredBackground", paletteLocked->ilistHovSelBackColor.name(QColor::HexArgb));
	settings.endGroup();

	settings.beginGroup("Oscilloscope");
	settings.setValue("background", paletteLocked->wavBackColor.name(QColor::HexArgb));
	settings.setValue("foreground", paletteLocked->wavDrawColor.name(QColor::HexArgb));
	settings.endGroup();
}

bool ColorPaletteHandler::loadPalette(std::weak_ptr<ColorPalette> palette)
{
	try {
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, ColorPaletteHandler::organization, ColorPaletteHandler::application);
		load(settings, palette);
		return true;
	} catch (...) {
		return false;
	}
}

bool ColorPaletteHandler::loadPalette(QString file, std::weak_ptr<ColorPalette> palette)
{
	try {
		QSettings settings(file, QSettings::IniFormat);
		load(settings, palette);
		return true;
	} catch (...) {
		return false;
	}
}

void ColorPaletteHandler::load(QSettings& settings, std::weak_ptr<ColorPalette> palette)
{
	std::shared_ptr<ColorPalette> paletteLocked = palette.lock();

	settings.beginGroup("PatternEditor");
	paletteLocked->ptnDefTextColor = settings.value("defaultStepText", paletteLocked->ptnDefTextColor).value<QColor>();
	paletteLocked->ptnDefStepColor = settings.value("defaultStepBackground", paletteLocked->ptnDefStepColor).value<QColor>();
	paletteLocked->ptnHl1StepColor = settings.value("highlightedStep1Background", paletteLocked->ptnHl1StepColor).value<QColor>();
	paletteLocked->ptnHl2StepColor = settings.value("highlightedStep2Background", paletteLocked->ptnHl2StepColor).value<QColor>();
	paletteLocked->ptnCurTextColor = settings.value("currentStepText", paletteLocked->ptnCurTextColor).value<QColor>();
	paletteLocked->ptnCurStepColor = settings.value("currentStepBackground", paletteLocked->ptnCurStepColor).value<QColor>();
	paletteLocked->ptnCurEditStepColor = settings.value("currentEditingStepBackground", paletteLocked->ptnCurEditStepColor).value<QColor>();
	paletteLocked->ptnCurCellColor = settings.value("currentCellBackground", paletteLocked->ptnCurCellColor).value<QColor>();
	paletteLocked->ptnPlayStepColor = settings.value("currentPlayingStepBackground", paletteLocked->ptnPlayStepColor).value<QColor>();
	paletteLocked->ptnSelCellColor = settings.value("selectionBackground", paletteLocked->ptnSelCellColor).value<QColor>();
	paletteLocked->ptnHovCellColor = settings.value("hoveredCellBackground", paletteLocked->ptnHovCellColor).value<QColor>();
	paletteLocked->ptnDefStepNumColor = settings.value("defaultStepNumber", paletteLocked->ptnDefStepNumColor).value<QColor>();
	paletteLocked->ptnHl1StepNumColor = settings.value("highlightedStep1Number", paletteLocked->ptnHl1StepNumColor).value<QColor>();
	paletteLocked->ptnHl2StepNumColor = settings.value("highlightedStep2Number", paletteLocked->ptnHl2StepNumColor).value<QColor>();
	paletteLocked->ptnNoteColor = settings.value("noteText", paletteLocked->ptnNoteColor).value<QColor>();
	paletteLocked->ptnInstColor = settings.value("instrumentText", paletteLocked->ptnInstColor).value<QColor>();
	paletteLocked->ptnVolColor = settings.value("volumeText", paletteLocked->ptnVolColor).value<QColor>();
	paletteLocked->ptnEffColor = settings.value("effectText", paletteLocked->ptnEffColor).value<QColor>();
	paletteLocked->ptnErrorColor = settings.value("errorText", paletteLocked->ptnErrorColor).value<QColor>();
	paletteLocked->ptnHeaderTextColor = settings.value("headerText", paletteLocked->ptnHeaderTextColor).value<QColor>();
	paletteLocked->ptnHeaderRowColor = settings.value("headerBackground", paletteLocked->ptnHeaderRowColor).value<QColor>();
	paletteLocked->ptnMaskColor = settings.value("mask", paletteLocked->ptnMaskColor).value<QColor>();
	paletteLocked->ptnBorderColor = settings.value("border", paletteLocked->ptnBorderColor).value<QColor>();
	paletteLocked->ptnMuteColor = settings.value("mute", paletteLocked->ptnMuteColor).value<QColor>();
	paletteLocked->ptnUnmuteColor = settings.value("unmute", paletteLocked->ptnUnmuteColor).value<QColor>();
	paletteLocked->ptnBackColor = settings.value("background", paletteLocked->ptnBackColor).value<QColor>();
	settings.endGroup();

	settings.beginGroup("OrderList");
	paletteLocked->odrDefTextColor = settings.value("defaultRowText", paletteLocked->odrDefTextColor).value<QColor>();
	paletteLocked->odrDefRowColor = settings.value("defaultRowBackground", paletteLocked->odrDefRowColor).value<QColor>();
	paletteLocked->odrCurTextColor = settings.value("currentRowText", paletteLocked->odrCurTextColor).value<QColor>();
	paletteLocked->odrCurRowColor = settings.value("currentRowBackground", paletteLocked->odrCurRowColor).value<QColor>();
	paletteLocked->odrCurEditRowColor = settings.value("currentEditingRowBackground", paletteLocked->odrCurEditRowColor).value<QColor>();
	paletteLocked->odrCurCellColor = settings.value("currentCellBackground", paletteLocked->odrCurCellColor).value<QColor>();
	paletteLocked->odrPlayRowColor = settings.value("currentPlayingRowBackground", paletteLocked->odrPlayRowColor).value<QColor>();
	paletteLocked->odrSelCellColor = settings.value("selectionBackground", paletteLocked->odrSelCellColor).value<QColor>();
	paletteLocked->odrHovCellColor = settings.value("hoveredCellBackground", paletteLocked->odrHovCellColor).value<QColor>();
	paletteLocked->odrRowNumColor = settings.value("rowNumber", paletteLocked->odrRowNumColor).value<QColor>();
	paletteLocked->odrHeaderTextColor = settings.value("headerText", paletteLocked->odrHeaderTextColor).value<QColor>();
	paletteLocked->odrHeaderRowColor = settings.value("headerBackground", paletteLocked->odrHeaderRowColor).value<QColor>();
	paletteLocked->odrBorderColor = settings.value("border", paletteLocked->odrBorderColor).value<QColor>();
	paletteLocked->odrBackColor = settings.value("background", paletteLocked->odrBackColor).value<QColor>();
	settings.endGroup();

	settings.beginGroup("InstrumentList");
	paletteLocked->ilistTextColor = settings.value("defaultText", paletteLocked->ilistTextColor).value<QColor>();
	paletteLocked->ilistBackColor = settings.value("background", paletteLocked->ilistBackColor).value<QColor>();
	paletteLocked->ilistSelBackColor = settings.value("selectedBackground", paletteLocked->ilistSelBackColor).value<QColor>();
	paletteLocked->ilistHovBackColor = settings.value("hoveredBackground", paletteLocked->ilistHovBackColor).value<QColor>();
	paletteLocked->ilistHovSelBackColor = settings.value("selectedHoveredBackground", paletteLocked->ilistHovSelBackColor).value<QColor>();
	settings.endGroup();

	settings.beginGroup("Oscilloscope");
	paletteLocked->wavBackColor = settings.value("background", paletteLocked->wavBackColor).value<QColor>();
	paletteLocked->wavDrawColor = settings.value("foreground", paletteLocked->wavDrawColor).value<QColor>();
	settings.endGroup();
}
