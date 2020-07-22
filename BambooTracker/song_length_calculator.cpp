#include "song_length_calculator.hpp"
#include <unordered_set>
#include <unordered_map>
#include "effect.hpp"
#include "enum_hash.hpp"

SongLengthCalculator::SongLengthCalculator(Module& mod, int songNum)
	: mod_(mod), songNum_(songNum)
{
}

double SongLengthCalculator::calculateBySecond() const
{
	Song& song = mod_.getSong(songNum_);
	std::unordered_set<int> visitedOrder;
	double tickCnt = 0.;

	const int rate = static_cast<int>(mod_.getTickFrequency());
	int tempo = song.getTempo();
	int speed = song.getSpeed();
	std::vector<int> groove = mod_.getGroove(song.getGroove()).getSequence();
	double stepTicks = getStrictStepTicks(rate, tempo, speed);
	size_t grooveIdx = 0;
	bool isTempo = song.isUsedTempo();

	std::vector<TrackAttribute> attribs = song.getTrackAttributes();
	int orderNum = 0;
	int stepNum = 0;
	int maxOrder = static_cast<int>(song.getOrderSize());

	while (!visitedOrder.count(orderNum)) {
		visitedOrder.insert(orderNum);

		int maxStep = static_cast<int>(song.getPatternSizeFromOrderNumber(orderNum));
		std::unordered_map<EffectType, int> jumpEffMap;
		for (; stepNum < maxStep; ++stepNum) {
			std::unordered_map<EffectType, int> speedEffMap;
			// Load effects
			for (const TrackAttribute& attrib : attribs) {
				Step& step = song.getTrack(attrib.number).getPatternFromOrderNumber(orderNum).getStep(stepNum);
				for (int e = 0; e < 4; ++e) {
					const Effect&& eff = Effect::makeEffectData(attrib.source, step.getEffectID(e), step.getEffectValue(e));
					switch (eff.type) {
					case EffectType::SpeedTempoChange:
					case EffectType::Groove:
						speedEffMap[eff.type] = eff.value;
						break;
					case EffectType::PositionJump:
					case EffectType::SongEnd:
					case EffectType::PatternBreak:
						if (stepNum == maxStep - 1) jumpEffMap[eff.type] = eff.value;	// Read only last step
						break;
					default:
						break;
					}
				}
			}

			// Update playback state
			auto&& it = speedEffMap.find(EffectType::SpeedTempoChange);
			if (it != speedEffMap.end()) {
				if (it->second < 0x20 && speed != it->second) {
					speed = it->second;
					isTempo = true;
					stepTicks = getStrictStepTicks(rate, tempo, speed);
				}
				else if (tempo != it->second) {
					tempo = it->second;
					isTempo = true;
					stepTicks = getStrictStepTicks(rate, tempo, speed);
				}
			}
			for (const auto& eff : speedEffMap) {
				switch (eff.first) {
				case EffectType::Groove:
					if (eff.second < static_cast<int>(mod_.getGrooveCount())) {
						groove = mod_.getGroove(eff.second).getSequence();
						isTempo = false;
						grooveIdx = 0;
					}
					break;
				default:
					break;
				}
			}

			// Add step ticks
			if (isTempo) {
				tickCnt += stepTicks;
			}
			else {
				tickCnt += groove[grooveIdx];
				++grooveIdx %= groove.size();
			}
		}

		// Update order position
		++orderNum %= maxOrder;
		stepNum = 0;
		for (auto eff : jumpEffMap) {
			switch (eff.first) {
			case EffectType::PositionJump:
				if (eff.second < maxOrder) {
					orderNum = eff.second;
				}
				break;
			case EffectType::SongEnd:
				orderNum = 0;	// To break order loop
				break;
			case EffectType::PatternBreak:
				if (eff.second < static_cast<int>(song.getPatternSizeFromOrderNumber(orderNum))) {
					stepNum = eff.second;
				}
				break;
			default:
				break;
			}
		}
	}

	// Calculate time by seconds
	return tickCnt / rate;
}
