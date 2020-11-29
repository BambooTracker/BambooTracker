/*
 * Copyright (C) 2019-2020 Rerrah
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

#include "instrument_io.hpp"
#include "file_io_error.hpp"
#include "bti_io.hpp"
#include "dmp_io.hpp"
#include "tfi_io.hpp"
#include "vgi_io.hpp"
#include "opni_io.hpp"
#include "y12_io.hpp"
#include "ins_io.hpp"

namespace io
{
	AbstractInstrument* AbstractInstrumentIO::load(const BinaryContainer& ctr, const std::string& fileName,
									 std::weak_ptr<InstrumentsManager> instMan,
									 int instNum) const
	{
		(void)ctr;
		(void)fileName;
		(void)instMan;
		(void)instNum;
		throw FileUnsupportedError(FileType::Inst);
	}

	void AbstractInstrumentIO::save(BinaryContainer& ctr,
					  const std::weak_ptr<InstrumentsManager> instMan, int instNum) const
	{
		(void)ctr;
		(void)instMan;
		(void)instNum;
		throw FileUnsupportedError(FileType::Inst);
	}

	//------------------------------------------------------------

	std::unique_ptr<InstrumentIO> InstrumentIO::instance_;

	InstrumentIO::InstrumentIO()
	{
		handler_.add(new BtiIO);
		handler_.add(new DmpIO);
		handler_.add(new TfiIO);
		handler_.add(new VgiIO);
		handler_.add(new OpniIO);
		handler_.add(new Y12IO);
		handler_.add(new InsIO);
	}

	InstrumentIO& InstrumentIO::getInstance()
	{
		if (!instance_) instance_.reset(new InstrumentIO);
		return *instance_;
	}

	void InstrumentIO::saveInstrument(BinaryContainer& ctr,
									  const std::weak_ptr<InstrumentsManager> instMan, int instNum)
	{
		handler_.at("bti")->save(ctr, instMan, instNum);
	}

	AbstractInstrument* InstrumentIO::loadInstrument(const BinaryContainer& ctr, const std::string& path,
													 std::weak_ptr<InstrumentsManager> instMan,
													 int instNum)
	{
		size_t fnpos = path.find_last_of("/");
		std::string name = path.substr(fnpos + 1, path.find_last_of(".") - fnpos - 1);
		return handler_.at(getExtension(path))->load(ctr, name, instMan, instNum);
	}
}
