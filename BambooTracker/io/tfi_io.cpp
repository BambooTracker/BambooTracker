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
	size_t csr = 0;

	InstrumentFM* inst = new InstrumentFM(instNum, fileName, instManLocked.get());
	inst->setEnvelopeNumber(envIdx);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AL, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::FB, ctr.readUint8(csr++));

	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML1, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT1,
										  convertDtFromDmpTfiVgi(ctr.readUint8(csr++)));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL1, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS1, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR1, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR1, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR1, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR1, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL1, ctr.readUint8(csr++));
	int ssgeg1 = ctr.readUint8(csr++);
	ssgeg1 = ssgeg1 & 8 ? ssgeg1 & 7 : -1;
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG1, ssgeg1);

	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML2, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT2,
										  convertDtFromDmpTfiVgi(ctr.readUint8(csr++)));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL2, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS2, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR2, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR2, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR2, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR2, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL2, ctr.readUint8(csr++));
	int ssgeg2 = ctr.readUint8(csr++);
	ssgeg2 = ssgeg2 & 8 ? ssgeg2 & 7 : -1;
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG2, ssgeg2);

	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML3, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT3,
										  convertDtFromDmpTfiVgi(ctr.readUint8(csr++)));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL3, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS3, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR3, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR3, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR3, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR3, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL3, ctr.readUint8(csr++));
	int ssgeg3 = ctr.readUint8(csr++);
	ssgeg3 = ssgeg3 & 8 ? ssgeg1 & 7 : -1;
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG3, ssgeg3);

	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML4, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT4,
										  convertDtFromDmpTfiVgi(ctr.readUint8(csr++)));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL4, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS4, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR4, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR4, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR4, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR4, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL4, ctr.readUint8(csr++));
	int ssgeg4 = ctr.readUint8(csr++);
	ssgeg4 = ssgeg4 & 8 ? ssgeg4 & 7 : -1;
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG4, ssgeg4);
	return inst;
}
}
