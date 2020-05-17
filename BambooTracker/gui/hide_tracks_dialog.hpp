#ifndef HIDE_TRACKS_DIALOG_HPP
#define HIDE_TRACKS_DIALOG_HPP

#include <QDialog>
#include <vector>

struct SongStyle;

namespace Ui {
	class HideTracksDialog;
}

class HideTracksDialog : public QDialog
{
	Q_OBJECT

public:
	explicit HideTracksDialog(const SongStyle& style, const std::vector<int>& tracks,
							  QWidget *parent = nullptr);
	~HideTracksDialog() override;

	std::vector<int> getVisibleTracks() const;

private:
	Ui::HideTracksDialog *ui;
	int checkCounter_;
	bool loopCheck_;
};

#endif // HIDE_TRACKS_DIALOG_HPP
