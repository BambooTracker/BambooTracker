#pragma once

#include <vector>
#include <string>

class Song;

size_t calculateColumnSize(int beginTrack, int beginColumn, int endTrack, int endColumn);

std::vector<std::vector<std::string>> getPreviousCells(Song& song, size_t w, size_t h, int beginTrack,
													   int beginColumn, int beginOrder, int beginStep);

void restorePattern(Song& song, const std::vector<std::vector<std::string>>& cells, int beginTrack,
					int beginColumn, int beginOrder, int beginStep);
