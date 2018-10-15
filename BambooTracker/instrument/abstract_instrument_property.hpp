#pragma once

#include <memory>
#include <vector>

class AbstractInstrumentProperty
{
public:
	virtual ~AbstractInstrumentProperty() = default;

	void setNumber(int num);
	int getNumber() const;

	void registerUserInstrument(int instNum);
	void deregisterUserInstrument(int instNum);
	bool isUserInstrument() const;
	std::vector<int> getUserInstruments() const;

protected:
	explicit AbstractInstrumentProperty(int num);
	AbstractInstrumentProperty(const AbstractInstrumentProperty& other);

private:
	int num_;
	std::vector<int> users_;
};
