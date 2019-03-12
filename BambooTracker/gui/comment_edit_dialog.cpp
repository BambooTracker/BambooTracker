#include "comment_edit_dialog.hpp"
#include "ui_comment_edit_dialog.h"

CommentEditDialog::CommentEditDialog(QString comment, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::CommentEditDialog)
{
	ui->setupUi(this);
	setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);
	ui->plainTextEdit->setPlainText(comment);
}

CommentEditDialog::~CommentEditDialog()
{
	delete ui;
}

QString CommentEditDialog::getComment() const
{
	return ui->plainTextEdit->toPlainText();
}
