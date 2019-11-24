#pragma once

#include <unordered_map>

namespace std {
  template <class T>
  struct hash {
	static_assert(is_enum<T>::value, "...");
	size_t operator()(T x) const noexcept {
	  using type = typename underlying_type<T>::type;
	  return hash<type>{}(static_cast<type>(x));
	}
  };
}
