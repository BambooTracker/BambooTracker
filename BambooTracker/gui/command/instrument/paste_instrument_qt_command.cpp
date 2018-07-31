#include "paste_instrument_qt_command.h"
#include <QRegularExpression>

PasteInstrumentQtCommand::PasteInstrumentQtCommand(QListWidget *list, int row,
                                                   std::map<int, std::unique_ptr<QWidget>>& map,
                                                   QString oldStr, QString newStr, QUndoCommand *parent) :
    QUndoCommand(parent), list_(list), row_(row), map_(map)
{
    if (oldStr.startsWith("FM")) {
        QRegularExpression re("^.+:(.+),.+$", QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatch match = re.match(str);

    }
    else if (oldStr.startsWith("PSG")) {

    }
}
