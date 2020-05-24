#ifndef SWAP_TRACKS_DIALOG_HPP
#define SWAP_TRACKS_DIALOG_HPP

#include <QDialog>

struct SongStyle;

namespace Ui {
	class SwapTracksDialog;
}

class SwapTracksDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SwapTracksDialog(const SongStyle& style, QWidget *parent = nullptr);
	~SwapTracksDialog() override;

	int getTrack1() const;
	int getTrack2() const;

private:
	Ui::SwapTracksDialog *ui;
};

#endif // SWAP_TRACKS_DIALOG_HPP
