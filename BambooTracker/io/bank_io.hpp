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
#include "bank.hpp"
#include "file_io.hpp"
#include "binary_container.hpp"

namespace io
{
	class AbstractBankIO
	{
	public:
		AbstractBankIO(std::string ext, std::string desc, bool loadable, bool savable)
			: ext_(ext), desc_(desc), loadable_(loadable), savable_(savable) {}
		virtual ~AbstractBankIO() = default;
		virtual AbstractBank* load(const BinaryContainer& ctr) const;
		virtual void save(BinaryContainer& ctr, const std::weak_ptr<InstrumentsManager> instMan,
						  const std::vector<int>& instNums) const;
		inline std::string getExtension() const { return ext_; }
		inline std::string getFilterText() const { return desc_ + "(*." + ext_ + ")"; }
		inline bool isLoadable() const { return loadable_; }
		inline bool isSavable() const { return savable_; }

	private:
		std::string ext_, desc_;
		bool loadable_, savable_;
	};

	class BankIO
	{
	public:
		static BankIO& getInstance();

		void saveBank(BinaryContainer& ctr, const std::weak_ptr<InstrumentsManager> instMan,
					  const std::vector<int>& instNums);
		AbstractBank* loadBank(const BinaryContainer& ctr, const std::string& path);

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
		BankIO();

		static std::unique_ptr<BankIO> instance_;
		FileIOManagerMap<AbstractBankIO> handler_;

	public:
		static void extractADPCMSamples(const BinaryContainer& ctr, size_t addrPos, size_t sampOffs,
										int maxCnt, std::vector<int>& ids,
										std::vector<std::vector<uint8_t>>& samples);
	};
}
