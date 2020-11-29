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

InsIO::InsIO() : AbstractInstrumentIO("ins", "MVSTracker instrument", true, false) {}

AbstractInstrument* InsIO::load(const BinaryContainer& ctr, const std::string& fileName,
								std::weak_ptr<InstrumentsManager> instMan, int instNum) const
{
	(void)fileName;
	std::shared_ptr<InstrumentsManager> instManLocked = instMan.lock();
	size_t csr = 0;
	if (ctr.readString(csr, 4).compare("MVSI") != 0) throw FileCorruptionError(FileIO::FileType::Inst, csr);
	csr += 4;
	/*uint8_t fileVersion = */std::stoi(ctr.readString(csr++, 1));
	size_t nameCsr = 0;
	while (ctr.readChar(nameCsr++) != '\0')
		;
	std::string name = ctr.readString(csr, nameCsr - csr);
	csr = nameCsr;

	if (ctr.size() - csr != 25) throw FileCorruptionError(FileIO::FileType::Inst, csr);

	int envIdx = instManLocked->findFirstAssignableEnvelopeFM();
	if (envIdx < 0) throw FileCorruptionError(FileIO::FileType::Inst, csr);

	InstrumentFM* inst = new InstrumentFM(instNum, name, instManLocked.get());
	inst->setEnvelopeNumber(envIdx);
	uint8_t tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML1, 0x0f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT1, tmp >> 4);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML2, 0x0f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT2, tmp >> 4);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML3, 0x0f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT3, tmp >> 4);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML4, 0x0f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT4, tmp >> 4);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL1, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL2, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL3, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL4, ctr.readUint8(csr++));
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR1, 0x3f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS1, tmp >> 6);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR2, 0x3f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS2, tmp >> 6);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR3, 0x3f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS3, tmp >> 6);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR4, 0x3f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS4, tmp >> 6);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR1, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR2, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR3, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR4, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR1, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR2, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR3, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR4, ctr.readUint8(csr++));
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR1, 0x0f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL1, tmp >> 4);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR2, 0x0f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL2, tmp >> 4);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR3, 0x0f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL3, tmp >> 4);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR4, 0x0f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL4, tmp >> 4);

	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AL, 0x07 & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::FB, tmp >> 3);

	return inst;
}
