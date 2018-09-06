#pragma once

#include <memory>
#include <vector>

class AbstructInstrumentProperty
{
public:
	virtual ~AbstructInstrumentProperty() = default;

	void setNumber(int num);
	int getNumber() const;

	void registerUserInstrument(int instNum);
	void deregisterUserInstrument(int instNum);
	bool isUserInstrument() const;
	std::vector<int> getUserInstruments() const;

protected:
	explicit AbstructInstrumentProperty(int num);
	AbstructInstrumentProperty(const AbstructInstrumentProperty& other);

private:
	int num_;
	std::vector<int> users_;
};
