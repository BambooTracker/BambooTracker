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

#include "tfi_io.hpp"
#include "file_io_error.hpp"
#include "io_utils.hpp"

namespace io
{
TfiIO::TfiIO() : AbstractInstrumentIO("tfi", "TFM Music Maker instrument", true, false) {}

AbstractInstrument* TfiIO::load(const BinaryContainer& ctr, const std::string& fileName,
								std::weak_ptr<InstrumentsManager> instMan, int instNum) const
{
	std::shared_ptr<InstrumentsManager> instManLocked = instMan.lock();
	if (ctr.size() != 42) throw FileCorruptionError(FileType::Inst, 0);
	int envIdx = instManLocked->findFirstAssignableEnvelopeFM();
	if (envIdx < 0) throw FileCorruptionError(FileType::Inst, 0);

	InstrumentFM* inst = new InstrumentFM(instNum, fileName, instManLocked.get());
	inst->setEnvelopeNumber(envIdx);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AL, ctr.readUint8(0));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::FB, ctr.readUint8(1));

	size_t csr = 2;
	for (int op = 0; op < 4; ++op) {
		const auto& params = FM_OP_PARAMS[op];
		instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::ML), ctr.readUint8(csr++));
		instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::DT),
											  convertDtFromDmpTfiVgi(ctr.readUint8(csr++)));
		instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::TL), ctr.readUint8(csr++));
		instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::KS), ctr.readUint8(csr++));
		instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::AR), ctr.readUint8(csr++));
		instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::DR), ctr.readUint8(csr++));
		instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::SR), ctr.readUint8(csr++));
		instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::RR), ctr.readUint8(csr++));
		instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::SL), ctr.readUint8(csr++));
		int ssgeg = ctr.readUint8(csr++);
		ssgeg = ssgeg & 8 ? ssgeg & 7 : -1;
		instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::SSGEG), ssgeg);
	}
	return inst;
}
}
