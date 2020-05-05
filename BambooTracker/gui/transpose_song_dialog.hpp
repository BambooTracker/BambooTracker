#ifndef TRANSPOSE_SONG_DIALOG_HPP
#define TRANSPOSE_SONG_DIALOG_HPP

#include <vector>
#include <QDialog>
#include <QListWidgetItem>

namespace Ui {
	class TransposeSongDialog;
}

class TransposeSongDialog : public QDialog
{
	Q_OBJECT

public:
	explicit TransposeSongDialog(QWidget *parent = nullptr);
	~TransposeSongDialog() override;

	int getTransposeSeminotes() const;
	std::vector<int> getExcludeInstruments() const;

private:
	Ui::TransposeSongDialog *ui;
	QListWidgetItem* checks[128];
};

#endif // TRANSPOSE_SONG_DIALOG_HPP
