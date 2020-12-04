/*
 * Copyright (C) 2020 Rerrah
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

#include "y12_io.hpp"
#include "file_io_error.hpp"

namespace io
{
Y12IO::Y12IO() : AbstractInstrumentIO("y12", "Gens KMod dump", true, false) {}

AbstractInstrument* Y12IO::load(const BinaryContainer& ctr, const std::string& fileName,
								std::weak_ptr<InstrumentsManager> instMan,
								int instNum) const
{
	std::shared_ptr<InstrumentsManager> instManLocked = instMan.lock();
	if (ctr.size() != 128) throw FileCorruptionError(FileType::Inst, 0);
	int envIdx = instManLocked->findFirstAssignableEnvelopeFM();
	if (envIdx < 0) throw FileCorruptionError(FileType::Inst, 0);

	InstrumentFM* inst = new InstrumentFM(instNum, fileName, instManLocked.get());
	inst->setEnvelopeNumber(envIdx);
	size_t csr = 0;
	for (const int op : { 0, 2, 1, 3 }) {
		const auto& params = FM_OP_PARAMS[op];
		uint8_t tmp = ctr.readUint8(csr++);
		instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::ML), 0x0f & tmp);
		instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::DT), 0x07 & (tmp >> 4));
		tmp = ctr.readUint8(csr++);
		instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::TL), 0x7f & tmp);
		tmp = ctr.readUint8(csr++);
		instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::AR), 0x1f & tmp);
		instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::KS), tmp >> 6);
		tmp = ctr.readUint8(csr++);
		instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::DR), 0x1f & tmp);
		tmp = ctr.readUint8(csr++);
		instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::SR), 0x1f & tmp);
		tmp = ctr.readUint8(csr++);
		instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::RR), 0x0f & tmp);
		instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::SL), tmp >> 4);
		int ssgeg = ctr.readUint8(csr++);
		ssgeg = ssgeg & 8 ? ssgeg & 7 : -1;
		instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::SSGEG), ssgeg);
		csr += 9;
	}

	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AL, ctr.readUint8(0x40));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::FB, ctr.readUint8(0x41));

	return inst;
}
}
