#include "change_instrument_name_qt_command.hpp"
#include "command_id.hpp"
#include "gui/instrument_editor/instrument_editor_fm_form.hpp"
#include "gui/instrument_editor/instrument_editor_ssg_form.hpp"
#include "gui/instrument_editor/instrument_editor_adpcm_form.hpp"
#include "gui/instrument_list_misc.hpp"
#include "misc.hpp"

ChangeInstrumentNameQtCommand::ChangeInstrumentNameQtCommand(QListWidget *list, int num, int row, SoundSource src,
															 std::weak_ptr<InstrumentFormManager> formMan,
															 QString oldName, QString newName,
															 QUndoCommand *parent)
	: QUndoCommand(parent),
	  list_(list),
	  num_(num),
	  row_(row),
	  formMan_(formMan),
	  oldName_(oldName),
	  newName_(newName),
	  source_(src)
{}

void ChangeInstrumentNameQtCommand::redo()
{
	auto item = list_->item(row_);
	auto title = makeInstrumentListText(num_, newName_);
	item->setText(title);

	if (auto form = formMan_.lock()->getForm(num_).get()) {
		switch (source_) {
		case SoundSource::FM:
			qobject_cast<InstrumentEditorFMForm*>(form)->setWindowTitle(title);
			break;
		case SoundSource::SSG:
			qobject_cast<InstrumentEditorSSGForm*>(form)->setWindowTitle(title);
			break;
		case SoundSource::ADPCM:
			qobject_cast<InstrumentEditorADPCMForm*>(form)->setWindowTitle(title);
			break;
		default:
			break;
		}
	}
}

void ChangeInstrumentNameQtCommand::undo()
{
	auto item = list_->item(row_);
	auto title = makeInstrumentListText(num_, oldName_);
	item->setText(title);

	if (auto form = formMan_.lock()->getForm(num_).get()) {
		switch (source_) {
		case SoundSource::FM:
			qobject_cast<InstrumentEditorFMForm*>(form)->setWindowTitle(title);
			break;
		case SoundSource::SSG:
			qobject_cast<InstrumentEditorSSGForm*>(form)->setWindowTitle(title);
			break;
		case SoundSource::ADPCM:
			qobject_cast<InstrumentEditorADPCMForm*>(form)->setWindowTitle(title);
			break;
		default:
			break;
		}
	}
}

int ChangeInstrumentNameQtCommand::id() const
{
	return CommandId::ChangeInstrumentName;
}
