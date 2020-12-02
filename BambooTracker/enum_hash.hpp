#pragma once

#if (defined __GNUC__) && (!defined __clang__)
#if (__GNUC__ < 6) || ((__GNUC__ == 6) && (__GNUC_MINOR__ < 1))
// Unsupport std::hash with enum types before gcc 6.1.
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=60970

#include <unordered_map>

namespace std
{
template <class T>
struct hash {
	static_assert(is_enum<T>::value, "...");
	size_t operator()(T x) const noexcept {
		using type = typename underlying_type<T>::type;
		return hash<type>{}(static_cast<type>(x));
	}
};
}

#endif
#endif
