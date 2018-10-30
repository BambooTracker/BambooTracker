#include "interpolate_pattern_command.hpp"

InterpolatePatternCommand::InterpolatePatternCommand(std::weak_ptr<Module> mod,
													 int songNum, int beginTrack, int beginColmn,
													 int beginOrder, int beginStep,
													 int endTrack, int endColumn, int endStep)
	: mod_(mod),
	  song_(songNum),
	  bTrack_(beginTrack),
	  bCol_(beginColmn),
	  order_(beginOrder),
	  bStep_(beginStep),
	  eTrack_(endTrack),
	  eCol_(endColumn),
	  eStep_(endStep)
{
	auto& sng = mod.lock()->getSong(songNum);
	int s = beginStep;

	int h = endStep - beginStep + 1;
	int w = 0;
	int tr = endTrack;
	int cl = endColumn;
	while (true) {
		if (tr == beginTrack) {
			w += (cl - beginColmn + 1);
			break;
		}
		else {
			w += (cl + 1);
			cl = 10;
			--tr;
		}
	}

	for (size_t i = 0; i < h; ++i) {
		prevCells_.emplace_back();
		int t = beginTrack;
		int c = beginColmn;
		for (size_t j = 0; j < w; ++j) {
			switch (c) {
			case 0:
				prevCells_.at(i).push_back(std::to_string(
											   sng.getTrack(t).getPatternFromOrderNumber(beginOrder).getStep(s).getNoteNumber()));
				break;
			case 1:
				prevCells_.at(i).push_back(std::to_string(
											   sng.getTrack(t).getPatternFromOrderNumber(beginOrder).getStep(s).getInstrumentNumber()));
				break;
			case 2:
				prevCells_.at(i).push_back(std::to_string(
											   sng.getTrack(t).getPatternFromOrderNumber(beginOrder).getStep(s).getVolume()));
				break;
			case 3:
				prevCells_.at(i).push_back(
							sng.getTrack(t).getPatternFromOrderNumber(beginOrder).getStep(s).getEffectID(0));
				break;
			case 4:
				prevCells_.at(i).push_back(std::to_string(
											   sng.getTrack(t).getPatternFromOrderNumber(beginOrder).getStep(s).getEffectValue(0)));
				break;
			case 5:
				prevCells_.at(i).push_back(
							sng.getTrack(t).getPatternFromOrderNumber(beginOrder).getStep(s).getEffectID(1));
				break;
			case 6:
				prevCells_.at(i).push_back(std::to_string(
											   sng.getTrack(t).getPatternFromOrderNumber(beginOrder).getStep(s).getEffectValue(1)));
				break;
			case 7:
				prevCells_.at(i).push_back(
							sng.getTrack(t).getPatternFromOrderNumber(beginOrder).getStep(s).getEffectID(2));
				break;
			case 8:
				prevCells_.at(i).push_back(std::to_string(
											   sng.getTrack(t).getPatternFromOrderNumber(beginOrder).getStep(s).getEffectValue(2)));
				break;
			case 9:
				prevCells_.at(i).push_back(
							sng.getTrack(t).getPatternFromOrderNumber(beginOrder).getStep(s).getEffectID(3));
				break;
			case 10:
				prevCells_.at(i).push_back(std::to_string(
											   sng.getTrack(t).getPatternFromOrderNumber(beginOrder).getStep(s).getEffectValue(3)));
				break;
			}

			++c;
			t += (c / 11);
			c %= 11;
		}
		++s;
	}
}

void InterpolatePatternCommand::redo()
{
	auto& sng = mod_.lock()->getSong(song_);
	int div = prevCells_.size() - 1;
	if (!div) div = 1;

	int t = bTrack_;
	int c = bCol_;
	for (size_t i = 0; i < prevCells_.front().size(); ++i) {
		int s = bStep_;
		for (size_t j = 0; j < prevCells_.size(); ++j) {
			switch (c) {
			case 0:
			{
				auto& pattern = sng.getTrack(t).getPatternFromOrderNumber(order_);
				int a = pattern.getStep(bStep_).getNoteNumber();
				int b = pattern.getStep(eStep_).getNoteNumber();
				if (a > -1 && b > -1)
					pattern.getStep(s).setNoteNumber(a + (b - a) * j / div);
				break;
			}
			case 1:
			{
				auto& pattern = sng.getTrack(t).getPatternFromOrderNumber(order_);
				int a = pattern.getStep(bStep_).getInstrumentNumber();
				int b = pattern.getStep(eStep_).getInstrumentNumber();
				if (a > -1 && b > -1)
					pattern.getStep(s).setInstrumentNumber(a + (b - a) * j / div);
				break;
			}
			case 2:
			{
				auto& pattern = sng.getTrack(t).getPatternFromOrderNumber(order_);
				int a = pattern.getStep(bStep_).getVolume();
				int b = pattern.getStep(eStep_).getVolume();
				if (a > -1 && b > -1)
					pattern.getStep(s).setVolume(a + (b - a) * j / div);
				break;
			}
			case 3:
			{
				auto& pattern = sng.getTrack(t).getPatternFromOrderNumber(order_);
				std::string a = pattern.getStep(bStep_).getEffectID(0);
				std::string b = pattern.getStep(eStep_).getEffectID(0);
				if (a == b)
					sng.getTrack(t).getPatternFromOrderNumber(order_)
							.getStep(s).setEffectID(0, a);
				break;
			}
			case 4:
			{
				auto& pattern = sng.getTrack(t).getPatternFromOrderNumber(order_);
				int a = pattern.getStep(bStep_).getEffectValue(0);
				int b = pattern.getStep(eStep_).getEffectValue(0);
				if (a > -1 && b > -1)
					pattern.getStep(s).setEffectValue(0, a + (b - a) * j / div);
				break;
			}
			case 5:
			{
				auto& pattern = sng.getTrack(t).getPatternFromOrderNumber(order_);
				std::string a = pattern.getStep(bStep_).getEffectID(1);
				std::string b = pattern.getStep(eStep_).getEffectID(1);
				if (a == b)
					sng.getTrack(t).getPatternFromOrderNumber(order_)
							.getStep(s).setEffectID(1, a);
				break;
			}
			case 6:
			{
				auto& pattern = sng.getTrack(t).getPatternFromOrderNumber(order_);
				int a = pattern.getStep(bStep_).getEffectValue(1);
				int b = pattern.getStep(eStep_).getEffectValue(1);
				if (a > -1 && b > -1)
					pattern.getStep(s).setEffectValue(1, a + (b - a) * j / div);
				break;
			}
			case 7:
			{
				auto& pattern = sng.getTrack(t).getPatternFromOrderNumber(order_);
				std::string a = pattern.getStep(bStep_).getEffectID(2);
				std::string b = pattern.getStep(eStep_).getEffectID(2);
				if (a == b)
					sng.getTrack(t).getPatternFromOrderNumber(order_)
							.getStep(s).setEffectID(2, a);
				break;
			}
			case 8:
			{
				auto& pattern = sng.getTrack(t).getPatternFromOrderNumber(order_);
				int a = pattern.getStep(bStep_).getEffectValue(2);
				int b = pattern.getStep(eStep_).getEffectValue(2);
				if (a > -1 && b > -1)
					pattern.getStep(s).setEffectValue(2, a + (b - a) * j / div);
				break;
			}
			case 9:
			{
				auto& pattern = sng.getTrack(t).getPatternFromOrderNumber(order_);
				std::string a = pattern.getStep(bStep_).getEffectID(3);
				std::string b = pattern.getStep(eStep_).getEffectID(3);
				if (a == b)
					sng.getTrack(t).getPatternFromOrderNumber(order_)
							.getStep(s).setEffectID(3, a);
				break;
			}
			case 10:
			{
				auto& pattern = sng.getTrack(t).getPatternFromOrderNumber(order_);
				int a = pattern.getStep(bStep_).getEffectValue(3);
				int b = pattern.getStep(eStep_).getEffectValue(3);
				if (a > -1 && b > -1)
					pattern.getStep(s).setEffectValue(3, a + (b - a) * j / div);
				break;
			}
			}

			++s;
		}
		++c;
		t += (c / 11);
		c %= 11;
	}
}

void InterpolatePatternCommand::undo()
{
	auto& sng = mod_.lock()->getSong(song_);

	int s = bStep_;
	for (size_t i = 0; i < prevCells_.size(); ++i) {
		int t = bTrack_;
		int c = bCol_;
		for (size_t j = 0; j < prevCells_.at(i).size(); ++j) {
			switch (c) {
			case 0:
				sng.getTrack(t).getPatternFromOrderNumber(order_).getStep(s)
						.setNoteNumber(std::stoi(prevCells_.at(i).at(j)));
				break;
			case 1:
				sng.getTrack(t).getPatternFromOrderNumber(order_).getStep(s)
						.setInstrumentNumber(std::stoi(prevCells_.at(i).at(j)));
				break;
			case 2:
				sng.getTrack(t).getPatternFromOrderNumber(order_).getStep(s)
						.setVolume(std::stoi(prevCells_.at(i).at(j)));
				break;
			case 3:
				sng.getTrack(t).getPatternFromOrderNumber(order_).getStep(s)
						.setEffectID(0, prevCells_.at(i).at(j));
				break;
			case 4:
				sng.getTrack(t).getPatternFromOrderNumber(order_).getStep(s)
						.setEffectValue(0, std::stoi(prevCells_.at(i).at(j)));
				break;
			case 5:
				sng.getTrack(t).getPatternFromOrderNumber(order_).getStep(s)
						.setEffectID(1, prevCells_.at(i).at(j));
				break;
			case 6:
				sng.getTrack(t).getPatternFromOrderNumber(order_).getStep(s)
						.setEffectValue(1, std::stoi(prevCells_.at(i).at(j)));
				break;
			case 7:
				sng.getTrack(t).getPatternFromOrderNumber(order_).getStep(s)
						.setEffectID(2, prevCells_.at(i).at(j));
				break;
			case 8:
				sng.getTrack(t).getPatternFromOrderNumber(order_).getStep(s)
						.setEffectValue(2, std::stoi(prevCells_.at(i).at(j)));
				break;
			case 9:
				sng.getTrack(t).getPatternFromOrderNumber(order_).getStep(s)
						.setEffectID(3, prevCells_.at(i).at(j));
				break;
			case 10:
				sng.getTrack(t).getPatternFromOrderNumber(order_).getStep(s)
						.setEffectValue(3, std::stoi(prevCells_.at(i).at(j)));
				break;
			}

			++c;
			t += (c / 11);
			c %= 11;
		}

		++s;
	}
}

int InterpolatePatternCommand::getID() const
{
	return 0x37;
}
