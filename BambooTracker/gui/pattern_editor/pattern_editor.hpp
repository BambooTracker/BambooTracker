#ifndef PATTERN_EDITOR_HPP
#define PATTERN_EDITOR_HPP

#include <QFrame>
#include <QUndoStack>
#include <QEvent>
#include <memory>
#include "bamboo_tracker.hpp"

namespace Ui {
	class PatternEditor;
}

class PatternEditor : public QFrame
{
	Q_OBJECT

public:
	explicit PatternEditor(QWidget *parent = nullptr);
	~PatternEditor() override;
	void setCore(std::shared_ptr<BambooTracker> core);
	void setCommandStack(std::weak_ptr<QUndoStack> stack);

	void changeEditable();
	void updatePosition();

	void copySelectedCells();
	void cutSelectedCells();

signals:
	void currentTrackChanged(int num);
	void currentOrderChanged(int num, int max);

	void focusIn();
	void focusOut();
	void selected(bool isSelected);

protected:
	bool eventFilter(QObject *watched, QEvent *event) override;

public slots:
	void setCurrentTrack(int num);
	void setCurrentOrder(int num);

	void onOrderListEdited();
	void onDefaultPatternSizeChanged();

	void setPatternHighlightCount(int count);

	void onSongLoaded();

	void onDeletePressed();
	void onPastePressed();
	void onPasteMixPressed();
	/// 0: None
	/// 1: All
	void onSelectPressed(int type);
	void onTransposePressed(bool isOctave, bool isIncreased);
	void onMuteTrackPressed();
	void onSoloTrackPressed();

private:
	Ui::PatternEditor *ui;
	std::shared_ptr<BambooTracker> bt_;
};

#endif // PATTERN_EDITOR_HPP
