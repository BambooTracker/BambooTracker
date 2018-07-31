#ifndef PASTE_INSTRUMENT_QT_COMMAND_H
#define PASTE_INSTRUMENT_QT_COMMAND_H

#include <QUndoCommand>
#include <QListWidget>
#include <memory>
#include <map>
#include "misc.hpp"


class PasteInstrumentQtCommand : public QUndoCommand
{
public:
    PasteInstrumentQtCommand(QListWidget *list, int row,
                             std::map<int, std::unique_ptr<QWidget>>& map,
                             QString oldStr,
                             QString newStr, QUndoCommand* parent = nullptr);
    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;
    int id() const Q_DECL_OVERRIDE;

private:
    QListWidget* list_;
    int row_;
    std::map<int, std::unique_ptr<QWidget>>& map_;
    QString oldName_, newName_;
};

#endif // PASTE_INSTRUMENT_QT_COMMAND_H
