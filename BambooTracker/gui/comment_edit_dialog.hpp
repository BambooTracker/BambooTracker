#ifndef COMMENT_EDIT_DIALOG_HPP
#define COMMENT_EDIT_DIALOG_HPP

#include <QDialog>
#include <QString>

namespace Ui {
	class CommentEditDialog;
}

class CommentEditDialog : public QDialog
{
	Q_OBJECT

public:
	explicit CommentEditDialog(QString comment, QWidget *parent = nullptr);
	~CommentEditDialog();
	QString getComment() const;

private:
	Ui::CommentEditDialog *ui;
};

#endif // COMMENT_EDIT_DIALOG_HPP
