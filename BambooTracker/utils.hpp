/*
 * Copyright (C) 2018-2022 Rerrah
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <cmath>
#include <algorithm>
#include <type_traits>
#include <vector>
#include <iterator>

namespace utils
{
template <typename T>
inline T clamp(T value, T low, T high)
{
	return std::min(std::max(value, low), high);
}

template <class InputContainer, class T>
inline auto find(InputContainer& input, const T& value)
{
	return std::find(input.begin(), input.end(), value);
}

template <class InputContainer, class Predicate>
inline auto findIf(InputContainer& input, Predicate&& pred)
{
	return std::find_if(input.begin(), input.end(), std::forward<Predicate>(pred));
}

template <class OutputContainer = std::vector<int>, class InputIterator, class Predicate>
inline auto findIndicesIf(InputIterator first, InputIterator last, Predicate pred)
{
	using T = typename OutputContainer::value_type;
	OutputContainer idcs;
	for (auto it = first; (it = std::find_if(it, last, pred)) != last; ++it) {
		idcs.push_back(static_cast<T>(std::distance(first, it)));
	}
	return idcs;
}

template <class OutputContainer = std::vector<int>, class InputContainer, class Predicate>
inline auto findIndicesIf(const InputContainer& input, Predicate&& pred)
{
	return findIndicesIf<OutputContainer>(input.begin(), input.end(), std::forward<Predicate>(pred));
}

template <template <typename ...> class OutputContainer = std::vector,
		  class InputContainer, class UnaryOperation>
inline auto transform(InputContainer& input, UnaryOperation&& op)
{
	// std::result_of is deprecated in C++17 and obsoluted in C++20, use std::invoke_result after C++14
	using T = typename std::result_of<UnaryOperation&&(typename InputContainer::value_type)>::type;
	OutputContainer<T> output;
	std::transform(input.begin(), input.end(), std::back_inserter(output), op);
	return output;
}

template <class InputIterator, class OutputIterator, class Predicate, class UnaryOperation>
inline auto transformIf(InputIterator first, InputIterator last,
						OutputIterator result, Predicate pred, UnaryOperation op)
{
	for (auto it = first; (it = std::find_if(it, last, pred)) != last; it++, result++)
		result = op(*it);
	return result;
}

template <template <typename ...> class OutputContainer = std::vector,
		  class InputContainer, class Predicate, class UnaryOperation>
inline auto transformIf(const InputContainer& input, Predicate&& pred, UnaryOperation&& op)
{
	// std::result_of is deprecated in C++17 and obsoluted in C++20, use std::invoke_result after C++14
	using T = typename std::result_of<UnaryOperation&&(typename InputContainer::value_type)>::type;
	OutputContainer<T> output;
	transformIf(input.begin(), input.end(), std::back_inserter(output), pred, op);
	return output;
}

template <template <typename ...> class OutputContainer = std::vector, class Map>
inline auto getMapKeys(const Map& map)
{
	return transform<OutputContainer>(map, [](typename Map::const_reference pair) { return pair.first; });
}

template <class Map, typename T>
inline auto findMapValue(const Map& map, const T& value)
{
	return findIf(map, [&](typename Map::const_reference pair) { return pair.second == value; });
}

/**
 * @brief isInRange
 * @param value Value.
 * @param min  Left-closed value.
 * @param max Right-closed value.
 * @return /c True when /c value is in [lower, upper].
 */
template <typename T>
constexpr auto isInRange(T value, T lower, T upper)
{
	return (lower <= value && value <= upper);
}
}
