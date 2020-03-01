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
	void clearUserInstruments();

	virtual bool isEdited() const = 0;
	virtual void clearParameters() = 0;

protected:
	explicit AbstractInstrumentProperty(int num);
	AbstractInstrumentProperty(const AbstractInstrumentProperty& other);

private:
	int num_;
	std::vector<int> users_;
};
