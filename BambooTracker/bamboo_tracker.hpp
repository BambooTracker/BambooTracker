#pragma once

#include <QObject>
#include "opna.hpp"
#include "audio_stream.hpp"

class BambooTracker : public QObject
{
	Q_OBJECT

public:
	BambooTracker();

private:
	chip::OPNA chip_;
	AudioStream stream_;

private slots:
	void onNextStepArrived();
	void onNextTickArrived();
};
