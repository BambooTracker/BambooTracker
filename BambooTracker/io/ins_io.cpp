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

#include "ins_io.hpp"
#include "file_io_error.hpp"

namespace io
{
InsIO::InsIO() : AbstractInstrumentIO("ins", "MVSTracker instrument", true, false) {}

AbstractInstrument* InsIO::load(const BinaryContainer& ctr, const std::string& fileName,
								std::weak_ptr<InstrumentsManager> instMan, int instNum) const
{
	(void)fileName;
	std::shared_ptr<InstrumentsManager> instManLocked = instMan.lock();
	size_t csr = 0;
	if (ctr.readString(csr, 4).compare("MVSI") != 0) throw FileCorruptionError(FileType::Inst, csr);
	csr += 4;
	/*uint8_t fileVersion = */std::stoi(ctr.readString(csr++, 1));
	size_t nameCsr = 0;
	while (ctr.readChar(nameCsr++) != '\0')
		;
	std::string name = ctr.readString(csr, nameCsr - csr);
	csr = nameCsr;

	if (ctr.size() - csr != 25) throw FileCorruptionError(FileType::Inst, csr);

	int envIdx = instManLocked->findFirstAssignableEnvelopeFM();
	if (envIdx < 0) throw FileCorruptionError(FileType::Inst, csr);

	InstrumentFM* inst = new InstrumentFM(instNum, name, instManLocked.get());
	inst->setEnvelopeNumber(envIdx);
	uint8_t tmp;
	size_t initCsr[] = { 0, 1, 2, 3 };
	for (int op = 0; op < 4; ++op) {
		size_t pcsr = csr + initCsr[op];
		auto& params = FM_OP_PARAMS[op];
		tmp = ctr.readUint8(pcsr);
		instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::ML), 0x0f & tmp);
		instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::DT), tmp >> 4);
		pcsr += 4;
		instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::TL), ctr.readUint8(pcsr));
		pcsr += 4;
		tmp = ctr.readUint8(pcsr);
		instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::AR), 0x3f & tmp);
		instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::KS), tmp >> 6);
		pcsr += 4;
		instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::DR), ctr.readUint8(pcsr));
		pcsr += 4;
		instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::SR), ctr.readUint8(pcsr));
		pcsr += 4;
		tmp = ctr.readUint8(pcsr);
		instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::RR), 0x0f & tmp);
		instManLocked->setEnvelopeFMParameter(envIdx, params.at(FMOperatorParameter::SL), tmp >> 4);
	}
	csr += 24;
	tmp = ctr.readUint8(csr);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AL, 0x07 & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::FB, tmp >> 3);

	return inst;
}
}
