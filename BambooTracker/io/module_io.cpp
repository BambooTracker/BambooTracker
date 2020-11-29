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

#include "module_io.hpp"
#include "file_io_error.hpp"
#include "btm_io.hpp"

namespace io
{
	void AbstractModuleIO::load(const BinaryContainer& ctr, std::weak_ptr<Module> mod,
					  std::weak_ptr<InstrumentsManager> instMan) const
	{
		(void)ctr;
		(void)mod;
		(void)instMan;
		throw FileUnsupportedError(FileType::Mod);
	}

	void AbstractModuleIO::save(BinaryContainer& ctr, const std::weak_ptr<Module> mod,
					  const std::weak_ptr<InstrumentsManager> instMan) const
	{
		(void)ctr;
		(void)mod;
		(void)instMan;
		throw FileUnsupportedError(FileType::Mod);
	}

	//------------------------------------------------------------

	std::unique_ptr<ModuleIO> ModuleIO::instance_;

	ModuleIO::ModuleIO()
	{
		handler_.add(new BtmIO);
	}

	ModuleIO& ModuleIO::getInstance()
	{
		if (!instance_) instance_.reset(new ModuleIO);
		return *instance_;
	}

	void ModuleIO::saveModule(BinaryContainer& ctr, const std::weak_ptr<Module> mod,
							  const std::weak_ptr<InstrumentsManager> instMan)
	{
		handler_.at("btm")->save(ctr, mod, instMan);
	}

	void ModuleIO::loadModule(const BinaryContainer& ctr, std::weak_ptr<Module> mod,
							  std::weak_ptr<InstrumentsManager> instMan)
	{
		handler_.at("btm")->load(ctr, mod, instMan);
	}
}
