/*
 * SPDX-FileCopyrightText: 2025 Rerrah
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <algorithm>
#include <vector>

template <typename T>
class Vector2d
{
public:
	using row_type = typename std::vector<T>;

private:
	using content_type = std::vector<row_type>;

public:
	using size_type = typename content_type::size_type;
	using reference = typename content_type::reference;
	using const_reference = typename content_type::const_reference;
	using iterator = typename content_type::iterator;
	using const_iterator = typename content_type::const_iterator;
	using value_type = typename content_type::value_type;

	/**
	 * @brief Create an empty 2D-vector.
	 */
	Vector2d() noexcept {}

	Vector2d(size_type row, size_type column)
	{
		for (size_type i = 0; i < row; ++i) {
			vector_.emplace_back(column);
		}
	}

	Vector2d(const std::pair<size_type, size_type>& shape)
		: Vector2d(shape.first, shape.second) {}

	Vector2d(size_type row, size_type column, const T& value)
	{
		for (size_type i = 0; i < row; ++i) {
			vector_.emplace_back(column, value);
		}
	}

	Vector2d(const std::pair<size_type, size_type> shape, const T& value)
		: Vector2d(shape.first, shape.second, value) {}

	Vector2d(size_type row, size_type column, const row_type& values)
	{
		for (size_type i = 0; i < row; ++i) {
			vector_.emplace_back(values.begin() + i * column, values.begin() + (i + 1) * column);
		}
	}

	Vector2d(const std::pair<size_type, size_type>& shape, const row_type& values)
		: Vector2d(shape.first, shape.second, values) {}

	/**
	 * @return @c true when there are no rows with different number of columns.
	 */
	bool isValid() const
	{
		return std::all_of(vector_.cbegin() + 1, vector_.cend(), [colSize = columnSize()](const_reference row) { return row.size() == colSize; });
	}

	bool empty() const
	{
		return rowSize() == 0 && columnSize() == 0;
	}

	std::pair<size_type, size_type> shape() const
	{
		return { rowSize(), columnSize() };
	}

	size_type rowSize() const
	{
		return vector_.size();
	}

	size_type columnSize() const
	{
		return vector_.empty() ? 0 : vector_.front().size();
	}

	reference operator[](size_type n)
	{
		return vector_[n];
	}

	const_reference operator[](size_type n) const
	{
		return vector_[n];
	}

	reference at(size_type n)
	{
		return vector_.at(n);
	}

	const_reference at(size_type n) const
	{
		return vector_.at(n);
	}

	reference at(size_type row, size_type column)
	{
		return vector_.at(row).at(column);
	}

	const_reference at(size_type row, size_type column) const
	{
		return vector_.at(row).at(column);
	}

	iterator begin()
	{
		return vector_.begin();
	}

	const_iterator begin() const
	{
		return vector_.begin();
	}

	iterator end()
	{
		return vector_.end();
	}

	const_iterator end() const
	{
		return vector_.end();
	}

	const_iterator cbegin() const
	{
		return vector_.cbegin();
	}

	const_iterator cend() const
	{
		return vector_.cend();
	}

	Vector2d<T> clip(size_type beginRow, size_type beginColumn, size_type rowSize, size_type columnSize) const
	{
		size_type clippedbeginRow = std::min(beginRow, this->rowSize());
		size_type clippedbeginColumn = std::min(beginColumn, this->columnSize());

		size_type clippedEndRow = std::min(beginRow + rowSize, this->rowSize());
		size_type clippedEndColumn = std::min(beginColumn + columnSize, this->columnSize());

		size_type clippedRowSize = clippedEndRow - clippedbeginRow;
		size_type clippedColumnSize = clippedEndColumn - clippedbeginColumn;

		Vector2d<T> newData;
		for (size_type i = 0; i < clippedRowSize; ++i) {
			const auto copiedBeginIter = vector_[clippedbeginRow + i].begin() + clippedbeginColumn;
			newData.vector_.emplace_back(copiedBeginIter, copiedBeginIter + clippedColumnSize);
		}

		return newData;
	}

	Vector2d<T> clip(size_type beginRow, size_type beginColumn, std::pair<size_type, size_type> shape) const
	{
		return clip(beginRow, beginColumn, shape.first, shape.second);
	}

private:
	content_type vector_;
};
