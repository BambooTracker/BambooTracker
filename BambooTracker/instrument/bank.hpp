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
		   std::vector<BinaryContainer> instSecs, BinaryContainer propSec, uint32_t version);
	~BtBank() override;

	size_t getNumInstruments() const override;
	std::string getInstrumentIdentifier(size_t index) const override;
	std::string getInstrumentName(size_t index) const override;
	AbstractInstrument* loadInstrument(size_t index, std::weak_ptr<InstrumentsManager> instMan, int instNum) const override;

private:
	std::vector<BinaryContainer> instCtrs_;
	BinaryContainer propCtr_;
	std::vector<int> ids_;
	std::vector<std::string> names_;
	uint32_t version_;
};

class WopnBank : public AbstractBank
{
public:
	explicit WopnBank(WOPNFile *wopn);
	~WopnBank() override;

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

class RawFMBank : public AbstractBank
{
public:
	explicit RawFMBank(std::vector<int> ids, std::vector<std::string> names, std::vector<BinaryContainer> ctrs);

	size_t getNumInstruments() const override;
	std::string getInstrumentIdentifier(size_t index) const override;
	std::string getInstrumentName(size_t index) const override;
	AbstractInstrument* loadInstrument(size_t index, std::weak_ptr<InstrumentsManager> instMan, int instNum) const override;

	void setInstrumentName(size_t index, const std::string& name);

private:
	std::vector<int> ids_;
	std::vector<std::string> names_;
	std::vector<BinaryContainer> instCtrs_;
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
