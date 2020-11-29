/*
 * Copyright (C) 2018-2020 Rerrah
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

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <algorithm>
#include "instruments_manager.hpp"

namespace io
{
	enum class FileType
	{
		Mod, Inst, WAV, VGM, Bank, S98, Unknown
	};

	DECL_MAYBE_UNUSED
	inline std::string getExtension(const std::string& path)
	{
		std::string ext = path.substr(path.find_last_of(".") + 1);
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
		return ext;
	}

	template<class T>
	class FileIOManagerMap
	{
	public:
		void add(T* handler)
		{
			map_[handler->getExtension()].reset(handler);
			const std::string filter = handler->getFilterText();
			if (handler->isLoadable()) ldFilters_.push_back(filter);
			if (handler->isSavable()) svFilters_.push_back(filter);
		}
		inline bool containExtension(const std::string& ext) const { return map_.count(ext); }
		inline bool testLoadableExtension(const std::string& ext) const { return (map_.count(ext) && map_.at(ext)->isLoadable()); }
		inline bool testSavableExtension(const std::string& ext) const { return (map_.count(ext) && map_.at(ext)->isSavable()); }
		inline const std::unique_ptr<T>& at(std::string ext) const { return map_.at(ext); }
		inline std::vector<std::string> getLoadFilterList() const { return ldFilters_; }
		inline std::vector<std::string> getSaveFilterList() const { return svFilters_; }

	private:
		std::unordered_map<std::string, std::unique_ptr<T>> map_;
		std::vector<std::string> ldFilters_, svFilters_;
	};
}
