#ifndef PATTERN_EDITOR_HPP
#define PATTERN_EDITOR_HPP

#include <QFrame>
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

	void changeEditable();

signals:
	void currentTrackChanged(int num);
	void currentOrderChanged(int num);

public slots:
	void setCurrentTrack(int num);

private:
	Ui::PatternEditor *ui;
};

#endif // PATTERN_EDITOR_HPP
