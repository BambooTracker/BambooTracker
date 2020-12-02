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
	size_t csr = 0;

	InstrumentFM* inst = new InstrumentFM(instNum, fileName, instManLocked.get());
	inst->setEnvelopeNumber(envIdx);
	uint8_t tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML1, 0x0f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT1, 0x07 & (tmp >> 4));
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL1, 0x7f & tmp);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR1, 0x1f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS1, tmp >> 6);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR1, 0x1f & tmp);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR1, 0x1f & tmp);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR1, 0x0f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL1, tmp >> 4);
	int ssgeg1 = ctr.readUint8(csr++);
	ssgeg1 = ssgeg1 & 8 ? ssgeg1 & 7 : -1;
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG1, ssgeg1);
	csr += 9;

	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML3, 0x0f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT3, 0x07 & (tmp >> 4));
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL3, 0x7f & tmp);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR3, 0x1f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS3, tmp >> 6);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR3, 0x1f & tmp);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR3, 0x1f & tmp);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR3, 0x0f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL3, tmp >> 4);
	int ssgeg3 = ctr.readUint8(csr++);
	ssgeg3 = ssgeg3 & 8 ? ssgeg3 & 7 : -1;
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG3, ssgeg3);
	csr += 9;

	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML2, 0x0f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT2, 0x07 & (tmp >> 4));
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL2, 0x7f & tmp);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR2, 0x1f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS2, tmp >> 6);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR2, 0x1f & tmp);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR2, 0x1f & tmp);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR2, 0x0f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL2, tmp >> 4);
	int ssgeg2 = ctr.readUint8(csr++);
	ssgeg2 = ssgeg2 & 8 ? ssgeg2 & 7 : -1;
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG2, ssgeg2);
	csr += 9;

	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::ML4, 0x0f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DT4, 0x07 & (tmp >> 4));
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::TL4, 0x7f & tmp);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AR4, 0x1f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::KS4, tmp >> 6);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::DR4, 0x1f & tmp);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SR4, 0x1f & tmp);
	tmp = ctr.readUint8(csr++);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::RR4, 0x0f & tmp);
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SL4, tmp >> 4);
	int ssgeg4 = ctr.readUint8(csr++);
	ssgeg4 = ssgeg4 & 8 ? ssgeg4 & 7 : -1;
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::SSGEG4, ssgeg4);
	csr += 9;

	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::AL, ctr.readUint8(csr++));
	instManLocked->setEnvelopeFMParameter(envIdx, FMEnvelopeParameter::FB, ctr.readUint8(csr++));
	csr += 14;

	return inst;
}
}
