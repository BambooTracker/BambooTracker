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
	BtBank(BinaryContainer instSec, BinaryContainer propSec);
	~BtBank() override;

	size_t getNumInstruments() const override;
	std::string getInstrumentIdentifier(size_t index) const override;
	std::string getInstrumentName(size_t index) const override;
	AbstractInstrument* loadInstrument(size_t index, std::weak_ptr<InstrumentsManager> instMan, int instNum) const override;

private:
	BinaryContainer instCntr_, propCntr_;
	std::vector<int> ids_;
	std::vector<std::string> names_;
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
