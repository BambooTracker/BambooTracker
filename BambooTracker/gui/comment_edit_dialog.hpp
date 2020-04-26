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
	explicit CommentEditDialog(QString comment = "", QWidget *parent = nullptr);
	~CommentEditDialog() override;
	void setComment(QString text);

signals:
	void commentChanged(const QString& text);

private:
	Ui::CommentEditDialog *ui;

private slots:
	void on_plainTextEdit_textChanged();
};

#endif // COMMENT_EDIT_DIALOG_HPP
