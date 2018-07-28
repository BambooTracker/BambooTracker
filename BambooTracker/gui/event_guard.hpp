#ifndef EVENT_GUARD_HPP
#define EVENT_GUARD_HPP

namespace Ui {
	class EventGuard
	{
	public:
		explicit EventGuard(bool& isIgnoreEvent);
		~EventGuard();

	private:
		bool& isIgnoreEvent_;
	};
}

#endif // EVENT_GUARD_HPP
