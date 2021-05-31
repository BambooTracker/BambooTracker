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

#pragma once

#include <memory>
#include <string>
#include <vector>
#include "module.hpp"
#include "instruments_manager.hpp"
#include "binary_container.hpp"
#include "io_utils.hpp"

namespace io
{
class AbstractModuleIO
{
public:
	AbstractModuleIO(const std::string& ext, const std::string& desc, bool loadable, bool savable)
		: ext_(ext), desc_(desc), loadable_(loadable), savable_(savable) {}
	virtual ~AbstractModuleIO() = default;
	virtual void load(const BinaryContainer& ctr, std::weak_ptr<Module> mod,
					  std::weak_ptr<InstrumentsManager> instMan) const;
	virtual void save(BinaryContainer& ctr, const std::weak_ptr<Module> mod,
					  const std::weak_ptr<InstrumentsManager> instMan) const;
	inline std::string getExtension() const noexcept { return ext_; }
	inline std::string getFilterText() const { return desc_ + " (*." + ext_ + ")"; }
	inline bool isLoadable() const noexcept { return loadable_; }
	inline bool isSavable() const noexcept { return savable_; }

private:
	const std::string ext_, desc_;
	bool loadable_, savable_;
};

class ModuleIO
{
public:
	static ModuleIO& getInstance();

	void saveModule(BinaryContainer& ctr, const std::weak_ptr<Module> mod,
					const std::weak_ptr<InstrumentsManager> instMan);
	void loadModule(const BinaryContainer& ctr, std::weak_ptr<Module> mod,
					std::weak_ptr<InstrumentsManager> instMan);

	inline bool testLoadableFormat(const std::string& ext) const
	{
		return handler_.testLoadableExtension(ext);
	}

	inline bool testSavableFormat(const std::string& ext) const
	{
		return handler_.testSavableExtension(ext);
	}

	inline std::vector<std::string> getLoadFilter() const
	{
		return handler_.getLoadFilterList();
	}

	inline std::vector<std::string> getSaveFilter() const
	{
		return handler_.getSaveFilterList();
	}

private:
	ModuleIO();

	static std::unique_ptr<ModuleIO> instance_;
	FileIOHandlerMap<AbstractModuleIO> handler_;
};
}
