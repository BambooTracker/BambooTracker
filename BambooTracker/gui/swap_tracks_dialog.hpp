#ifndef SWAP_TRACKS_DIALOG_HPP
#define SWAP_TRACKS_DIALOG_HPP

#include <QDialog>

namespace Ui {
	class SwapTracksDialog;
}

class SwapTracksDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SwapTracksDialog(QWidget *parent = nullptr);
	~SwapTracksDialog();

private:
	Ui::SwapTracksDialog *ui;
};

#endif // SWAP_TRACKS_DIALOG_HPP
