/*
 * Copyright (C) 2020-2022 Rerrah
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

#include "song_length_calculator.hpp"
#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <vector>
#include "module.hpp"
#include "effect.hpp"
#include "enum_hash.hpp"

namespace
{
inline double calculateStrictStepTicks(int rate, int tempo, int speed)
{
	return 2.5 * rate * speed / tempo;
}
}

SongLengthCalculator::SongLengthCalculator(Module& mod, int songNum)
	: mod_(mod), songNum_(songNum)
{
}

double SongLengthCalculator::approximateLengthBySecond() const
{
	Song& song = mod_.getSong(songNum_);
	std::unordered_set<int> visitedOrder;
	double tickCnt = 0.;

	const int rate = static_cast<int>(mod_.getTickFrequency());
	int tempo = song.getTempo();
	int speed = song.getSpeed();
	std::vector<int> groove = mod_.getGroove(song.getGroove());
	double stepTicks = calculateStrictStepTicks(rate, tempo, speed);
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
				for (int e = 0; e < Step::N_EFFECT; ++e) {
					const Effect&& eff = effect_utils::validateEffect(attrib.source, step.getEffect(e));
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
				if (it->second < 0x20) {
					if (speed != it->second) {
						speed = it->second;
						isTempo = true;
						stepTicks = calculateStrictStepTicks(rate, tempo, speed);
					}
				}
				else if (tempo != it->second) {
					tempo = it->second;
					isTempo = true;
					stepTicks = calculateStrictStepTicks(rate, tempo, speed);
				}
			}
			for (const auto& eff : speedEffMap) {
				switch (eff.first) {
				case EffectType::Groove:
					if (eff.second < static_cast<int>(mod_.getGrooveCount())) {
						groove = mod_.getGroove(eff.second);
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

void SongLengthCalculator::totalStepCount(size_t &introSize, size_t &loopSize) const
{
	Song& song = mod_.getSong(songNum_);
	std::vector<TrackAttribute> attribs = song.getTrackAttributes();
	size_t totalStepCnt = 0;
	std::unordered_map<int, size_t> stepCntLogMap;
	int lastOrder = static_cast<int>(song.getOrderSize()) - 1;
	int curOrder = 0;

	for (int curStep = 0; !stepCntLogMap.count(curOrder); ) {
		// Count up
		size_t ptnFullSize = song.getPatternSizeFromOrderNumber(curOrder);
		stepCntLogMap[curOrder] = totalStepCnt;
		totalStepCnt += (ptnFullSize - curStep);

		// Check next order position
		const int odr = std::exchange(curOrder, (curOrder + 1) % (lastOrder + 1));
		curStep = 0;
		for (const auto& attrib : attribs) {
			const Step& step = song.getTrack(attrib.number)
							   .getPatternFromOrderNumber(odr).getStep(ptnFullSize - 1);
			for (int i = 0; i < Step::N_EFFECT; ++i) {
				Effect&& eff = effect_utils::validateEffect(attrib.source, step.getEffect(i));
				switch (eff.type) {
				case EffectType::PositionJump:
					if (eff.value <= lastOrder) {
						curOrder = eff.value;
						curStep = 0;
					}
					break;
				case EffectType::SongEnd:	// No loop
					introSize = totalStepCnt;
					loopSize = 0;
					return;
				case EffectType::PatternBreak:
					if (eff.value < static_cast<int>(song.getPatternSizeFromOrderNumber(curOrder))) {
						curStep = eff.value;
					}
					break;
				default:
					break;
				}
			}
		}
	}

	// Calculate counts
	introSize = stepCntLogMap[curOrder];
	loopSize = totalStepCnt - introSize;
}
