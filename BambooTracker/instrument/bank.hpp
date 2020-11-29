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

#include <memory>
#include <vector>
#include "io/binary_container.hpp"

class AbstractInstrument;
class InstrumentsManager;
struct WOPNFile;

class AbstractBank
{
public:
	virtual ~AbstractBank() = default;

	virtual size_t getNumInstruments() const = 0;
	virtual std::string getInstrumentIdentifier(size_t index) const = 0;
	virtual std::string getInstrumentName(size_t index) const = 0;
	virtual AbstractInstrument* loadInstrument(size_t index, std::weak_ptr<InstrumentsManager> instMan, int instNum) const = 0;
};

class BtBank : public AbstractBank
{
public:
	BtBank(std::vector<int> ids, std::vector<std::string> names);
	BtBank(std::vector<int> ids, std::vector<std::string> names,
		   std::vector<io::BinaryContainer> instSecs, io::BinaryContainer propSec, uint32_t version);

	size_t getNumInstruments() const override;
	std::string getInstrumentIdentifier(size_t index) const override;
	std::string getInstrumentName(size_t index) const override;
	AbstractInstrument* loadInstrument(size_t index, std::weak_ptr<InstrumentsManager> instMan, int instNum) const override;

private:
	std::vector<io::BinaryContainer> instCtrs_;
	io::BinaryContainer propCtr_;
	std::vector<int> ids_;
	std::vector<std::string> names_;
	uint32_t version_;
};

class WopnBank : public AbstractBank
{
public:
	explicit WopnBank(WOPNFile *wopn);

	size_t getNumInstruments() const override;
	std::string getInstrumentIdentifier(size_t index) const override;
	std::string getInstrumentName(size_t index) const override;
	AbstractInstrument* loadInstrument(size_t index, std::weak_ptr<InstrumentsManager> instMan, int instNum) const override;

private:
	struct WOPNDeleter {
		void operator()(WOPNFile *x);
	};
	std::unique_ptr<WOPNFile, WOPNDeleter> wopn_;

	struct InstEntry;
	std::vector<InstEntry> entries_;
};

class FfBank : public AbstractBank
{
public:
	explicit FfBank(std::vector<int> ids, std::vector<std::string> names, std::vector<io::BinaryContainer> ctrs);

	size_t getNumInstruments() const override;
	std::string getInstrumentIdentifier(size_t index) const override;
	std::string getInstrumentName(size_t index) const override;
	AbstractInstrument* loadInstrument(size_t index, std::weak_ptr<InstrumentsManager> instMan, int instNum) const override;

	void setInstrumentName(size_t index, const std::string& name);

private:
	std::vector<int> ids_;
	std::vector<std::string> names_;
	std::vector<io::BinaryContainer> instCtrs_;
};

class PpcBank : public AbstractBank
{
public:
	explicit PpcBank(std::vector<int> ids, std::vector<std::vector<uint8_t>> samples);

	size_t getNumInstruments() const override;
	std::string getInstrumentIdentifier(size_t index) const override;
	std::string getInstrumentName(size_t index) const override;
	AbstractInstrument* loadInstrument(size_t index, std::weak_ptr<InstrumentsManager> instMan, int instNum) const override;

private:
	std::vector<int> ids_;
	std::vector<std::vector<uint8_t>> samples_;
};

class PviBank : public AbstractBank
{
public:
	explicit PviBank(std::vector<int> ids, std::vector<std::vector<uint8_t>> samples);

	size_t getNumInstruments() const override;
	std::string getInstrumentIdentifier(size_t index) const override;
	std::string getInstrumentName(size_t index) const override;
	AbstractInstrument* loadInstrument(size_t index, std::weak_ptr<InstrumentsManager> instMan, int instNum) const override;

private:
	std::vector<int> ids_;
	std::vector<std::vector<uint8_t>> samples_;
};

class Mucom88Bank : public AbstractBank
{
public:
	explicit Mucom88Bank(std::vector<int> ids, std::vector<std::string> names, std::vector<io::BinaryContainer> ctrs);

	size_t getNumInstruments() const override;
	std::string getInstrumentIdentifier(size_t index) const override;
	std::string getInstrumentName(size_t index) const override;
	AbstractInstrument* loadInstrument(size_t index, std::weak_ptr<InstrumentsManager> instMan, int instNum) const override;

	void setInstrumentName(size_t index, const std::string& name);

private:
	std::vector<int> ids_;
	std::vector<std::string> names_;
	std::vector<io::BinaryContainer> instCtrs_;
};
