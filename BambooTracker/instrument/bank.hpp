/*
 * Copyright (C) 2018-2021 Rerrah
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
#include <string>
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

class BtBank final : public AbstractBank
{
public:
	BtBank(const std::vector<int>& ids, const std::vector<std::string>& names);
	BtBank(const std::vector<int>& ids, const std::vector<std::string>& names,
		   const std::vector<io::BinaryContainer>& instSecs, const io::BinaryContainer& propSec, uint32_t version);

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

class WopnBank final : public AbstractBank
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

class FfBank final : public AbstractBank
{
public:
	FfBank(const std::vector<int>& ids, const std::vector<std::string>& names, const std::vector<io::BinaryContainer>& ctrs);

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

class PpcBank final : public AbstractBank
{
public:
	PpcBank(const std::vector<int>& ids, const std::vector<std::vector<uint8_t>>& samples);

	size_t getNumInstruments() const override;
	std::string getInstrumentIdentifier(size_t index) const override;
	std::string getInstrumentName(size_t) const override;
	AbstractInstrument* loadInstrument(size_t index, std::weak_ptr<InstrumentsManager> instMan, int instNum) const override;

private:
	std::vector<int> ids_;
	std::vector<std::vector<uint8_t>> samples_;
};

class P86Bank final : public AbstractBank
{
public:
	P86Bank(const std::vector<int>& ids, const std::vector<std::vector<uint8_t>>& samples);

	size_t getNumInstruments() const override;
	std::string getInstrumentIdentifier(size_t index) const override;
	std::string getInstrumentName(size_t) const override;
	AbstractInstrument* loadInstrument(size_t index, std::weak_ptr<InstrumentsManager> instMan, int instNum) const override;

private:
	std::vector<int> ids_;
	std::vector<std::vector<uint8_t>> samples_;
};

class PpsBank final : public AbstractBank
{
public:
	PpsBank(const std::vector<int>& ids, const std::vector<std::vector<uint8_t>>& samples);

	size_t getNumInstruments() const override;
	std::string getInstrumentIdentifier(size_t index) const override;
	std::string getInstrumentName(size_t) const override;
	AbstractInstrument* loadInstrument(size_t index, std::weak_ptr<InstrumentsManager> instMan, int instNum) const override;

private:
	std::vector<int> ids_;
	std::vector<std::vector<uint8_t>> samples_;
};

class PviBank final : public AbstractBank
{
public:
	PviBank(const std::vector<int>& ids, uint16_t deltaN, const std::vector<std::vector<uint8_t>>& samples);

	size_t getNumInstruments() const override;
	std::string getInstrumentIdentifier(size_t index) const override;
	std::string getInstrumentName(size_t) const override;
	AbstractInstrument* loadInstrument(size_t index, std::weak_ptr<InstrumentsManager> instMan, int instNum) const override;

private:
	std::vector<int> ids_;
	uint16_t deltaN_;
	std::vector<std::vector<uint8_t>> samples_;
};

class PziBank final : public AbstractBank
{
public:
	PziBank(const std::vector<int>& ids, const std::vector<int>& deltaNs,
			const std::vector<bool>& isRepeatedList, const std::vector<std::vector<uint8_t>>& samples);

	size_t getNumInstruments() const override;
	std::string getInstrumentIdentifier(size_t index) const override;
	std::string getInstrumentName(size_t) const override;
	AbstractInstrument* loadInstrument(size_t index, std::weak_ptr<InstrumentsManager> instMan, int instNum) const override;

private:
	std::vector<int> ids_;
	std::vector<int> deltaNs_;
	std::vector<bool> isRepeatedList_;
	std::vector<std::vector<uint8_t>> samples_;
};

class Mucom88Bank final : public AbstractBank
{
public:
	Mucom88Bank(const std::vector<int>& ids, const std::vector<std::string>& names, const std::vector<io::BinaryContainer>& ctrs);

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
