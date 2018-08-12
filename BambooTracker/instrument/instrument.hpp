#pragma once

#include <string>
#include <memory>
#include <map>
#include "instruments_manager.hpp"
#include "misc.hpp"

class InstrumentsManager;

class AbstructInstrument
{
public:
	virtual ~AbstructInstrument() = default;

	int getNumber() const;
	void setNumber(int n);
	SoundSource getSoundSource() const;
	std::string getName() const;
	void setName(std::string name);
	virtual std::unique_ptr<AbstructInstrument> clone() = 0;


protected:
	InstrumentsManager* owner_;
    std::string name_;	// UTF-8
	AbstructInstrument(int number, SoundSource source, std::string name, InstrumentsManager* owner);

private:
	int number_;
    SoundSource source_;
};


enum class FMParameter;

class InstrumentFM : public AbstructInstrument
{
public:
	InstrumentFM(int number, std::string name, InstrumentsManager* owner);
	std::unique_ptr<AbstructInstrument> clone() override;

	void setEnvelopeNumber(int n);
	int getEnvelopeNumber() const;
	int getEnvelopeParameter(FMParameter param) const;
	bool getOperatorEnable(int n) const;

private:
	int envNum_;
};

enum class FMParameter
{
	AL, FB,
	AR1, DR1, SR1, RR1, SL1, TL1, KS1, ML1, DT1, AM1,
	AR2, DR2, SR2, RR2, SL2, TL2, KS2, ML2, DT2, AM2,
	AR3, DR3, SR3, RR3, SL3, TL3, KS3, ML3, DT3, AM3,
	AR4, DR4, SR4, RR4, SL4, TL4, KS4, ML4, DT4, AM4,
	SSGEG1, SSGEG2, SSGEG3, SSGEG4
};


class InstrumentSSG : public AbstructInstrument
{
public:
	InstrumentSSG(int number, std::string name, InstrumentsManager* owner);
	std::unique_ptr<AbstructInstrument> clone() override;
};
