#include "bamboo_tracker.hpp"

BambooTracker::BambooTracker() :
	#ifdef SINC_INTERPOLATION
	chip_(3993600 * 2, 44100, 40),
	#else
	chip_(3993600 * 2, 44100),
	#endif
	stream_(chip_, chip_.getRate(), 40)
{
	QObject::connect(&stream_, &AudioStream::nextStepArrived,
					 this, &BambooTracker::onNextStepArrived, Qt::DirectConnection);
	QObject::connect(&stream_, &AudioStream::nextTickArrived,
					 this, &BambooTracker::onNextTickArrived, Qt::DirectConnection);
}

void BambooTracker::onNextStepArrived()
{

}

void BambooTracker::onNextTickArrived()
{

}
