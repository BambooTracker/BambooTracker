#ifndef PATTERN_EDITOR_HPP
#define PATTERN_EDITOR_HPP

#include <QFrame>
#include <QUndoStack>
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
	~PatternEditor();
	void setCore(std::shared_ptr<BambooTracker> core);
	void setCommandStack(std::weak_ptr<QUndoStack> stack);

	void changeEditable();
	void updatePosition();

signals:
	void currentTrackChanged(int num);
	void currentOrderChanged(int num);

public slots:
	void setCurrentTrack(int num);
	void setCurrentOrder(int num);

	void onOrderListEdited();
	void onDefaultPatternSizeChanged();

	void onSongLoaded();

private:
	Ui::PatternEditor *ui;
	std::shared_ptr<BambooTracker> bt_;
};

#endif // PATTERN_EDITOR_HPP
