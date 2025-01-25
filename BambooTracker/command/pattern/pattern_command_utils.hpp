/*
 * SPDX-FileCopyrightText: 2020 Rerrah
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <string>
#include <memory>
#include "module.hpp"
#include "vector_2d.hpp"

namespace command_utils
{
inline Step& getStep(std::weak_ptr<Module> mod, int song, int track, int order, int step)
{
	return mod.lock()->getSong(song).getTrack(track)
			.getPatternFromOrderNumber(order).getStep(step);
}

inline Step& getStep(Song& song, int track, int order, int step)
{
	return song.getTrack(track).getPatternFromOrderNumber(order).getStep(step);
}

inline Pattern& getPattern(std::weak_ptr<Module> mod, int song, int track, int order)
{
	return mod.lock()->getSong(song).getTrack(track).getPatternFromOrderNumber(order);
}

inline Pattern& getPattern(Song& song, int track, int order)
{
	return song.getTrack(track).getPatternFromOrderNumber(order);
}

size_t calculateColumnSize(int beginTrack, int beginColumn, int endTrack, int endColumn);

Vector2d<std::string> getPreviousCells(Song& song, std::size_t w, std::size_t h, int beginTrack,
                                       int beginColumn, int beginOrder, int beginStep);

/**
 * @throw @c std::invalid_argument or @c std::out_of_range if @c cells contain invalid data.
 */
void restorePattern(Song& song, const Vector2d<std::string>& cells, int beginTrack,
					int beginColumn, int beginOrder, int beginStep);
}
