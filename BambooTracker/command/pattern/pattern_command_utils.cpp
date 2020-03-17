#include "pattern_command_utils.hpp"
#include "song.hpp"

size_t calculateColumnSize(int beginTrack, int beginColumn, int endTrack, int endColumn)
{
	int w = 0;
	int tr = endTrack;
	int cl = endColumn;
	while (true) {
		if (tr == beginTrack) {
			w += (cl - beginColumn + 1);
			break;
		}
		else {
			w += (cl + 1);
			cl = 10;
			--tr;
		}
	}

	return static_cast<size_t>(w);
}

std::vector<std::vector<std::string>> getPreviousCells(Song& song, size_t w, size_t h, int beginTrack,
													   int beginColumn, int beginOrder, int beginStep)
{
	std::vector<std::vector<std::string>> cells(h);
	int s = beginStep;
	for (size_t i = 0; i < h; ++i) {
		int t = beginTrack;
		int c = beginColumn;
		cells[i].resize(w);
		for (size_t j = 0; j < w; ++j) {
			Step& st = song.getTrack(t).getPatternFromOrderNumber(beginOrder).getStep(s);
			std::string val;
			switch (c) {
			case 0:		val = std::to_string(st.getNoteNumber());		break;
			case 1:		val = std::to_string(st.getInstrumentNumber());	break;
			case 2:		val = std::to_string(st.getVolume());			break;
			case 3:		val = st.getEffectID(0);						break;
			case 4:		val = std::to_string(st.getEffectValue(0));		break;
			case 5:		val = st.getEffectID(1);						break;
			case 6:		val = std::to_string(st.getEffectValue(1));		break;
			case 7:		val = st.getEffectID(2);						break;
			case 8:		val = std::to_string(st.getEffectValue(2));		break;
			case 9:		val = st.getEffectID(3);						break;
			case 10:	val = std::to_string(st.getEffectValue(3));		break;
			}
			cells[i][j] = val;

			t += (++c / 11);
			c %= 11;
		}
		++s;
	}
	return cells;
}

void restorePattern(Song& song, const std::vector<std::vector<std::string>>& cells, int beginTrack,
					int beginColumn, int beginOrder, int beginStep)
{
	int s = beginStep;
	for (size_t i = 0; i < cells.size(); ++i) {
		int t = beginTrack;
		int c = beginColumn;
		for (size_t j = 0; j < cells[i].size(); ++j) {
			Step& st = song.getTrack(t).getPatternFromOrderNumber(beginOrder).getStep(s);
			switch (c) {
			case 0:		st.setNoteNumber(std::stoi(cells[i][j]));		break;
			case 1:		st.setInstrumentNumber(std::stoi(cells[i][j]));	break;
			case 2:		st.setVolume(std::stoi(cells[i][j]));			break;
			case 3:		st.setEffectID(0, cells[i][j]);					break;
			case 4:		st.setEffectValue(0, std::stoi(cells[i][j]));	break;
			case 5:		st.setEffectID(1, cells[i][j]);					break;
			case 6:		st.setEffectValue(1, std::stoi(cells[i][j]));	break;
			case 7:		st.setEffectID(2, cells[i][j]);					break;
			case 8:		st.setEffectValue(2, std::stoi(cells[i][j]));	break;
			case 9:		st.setEffectID(3, cells[i][j]);					break;
			case 10:	st.setEffectValue(3, std::stoi(cells[i][j]));	break;
			}

			t += (++c / 11);
			c %= 11;
		}
		++s;
	}
}
