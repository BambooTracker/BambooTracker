#include "event_guard.hpp"

namespace Ui {
	EventGuard::EventGuard(bool& isIgnoreEvent) :
		isIgnoreEvent_(isIgnoreEvent)
	{
		isIgnoreEvent_ = true;
	}

	EventGuard::~EventGuard()
	{
		isIgnoreEvent_ = false;
	}
}
