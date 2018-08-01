#ifndef PASTE_INSTRUMENT_QT_COMMAND_H
#define PASTE_INSTRUMENT_QT_COMMAND_H

#include <QUndoCommand>
#include <QListWidget>
#include <QString>
#include <memory>
#include <map>
#include "misc.hpp"

class PasteInstrumentQtCommand : public QUndoCommand
{
public:
	PasteInstrumentQtCommand(QListWidget *list, int oldRow, int refRow, int oldNum, int refNum,
							 std::map<int, std::unique_ptr<QWidget>>& map, SoundSource source,
							 QUndoCommand* parent = nullptr);
    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;
    int id() const Q_DECL_OVERRIDE;

private:
    QListWidget* list_;
	int oldRow_, refRow_;
	int oldNum_, refNum_;
	QString oldName_, refName_;
    std::map<int, std::unique_ptr<QWidget>>& map_;
	SoundSource source_;
};

#endif // PASTE_INSTRUMENT_QT_COMMAND_H
