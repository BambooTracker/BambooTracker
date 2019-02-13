#pragma once

#include <memory>
#include <vector>

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

class WopnBank : public AbstractBank
{
public:
	explicit WopnBank(WOPNFile *wopn);
	~WopnBank();

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
