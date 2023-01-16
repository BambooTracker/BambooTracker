/*
 * Copyright (C) 2023 Rerrah
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

#include "adpcm_address_spin_box.hpp"
#include <QIntValidator>

namespace
{
constexpr int ADDR_SHIFT = 5;
}

namespace
{
namespace start
{
constexpr int addressToPosition(int a)
{
	return a << (ADDR_SHIFT + 1);
}

constexpr int bytesToPosition(int b)
{
	return addressToPosition(b >> ADDR_SHIFT);
}

constexpr int positionToAddress(int p)
{
	return p >> (ADDR_SHIFT + 1);
}
}

class StartAddressValidator : public QIntValidator
{
public:
	StartAddressValidator(int bottom, int top, QObject *parent = nullptr) : QIntValidator(bottom, top, parent) {}
	QValidator::State validate(QString& input, int& pos) const override
	{
		auto state = QIntValidator::validate(input, pos);
		if (state == QIntValidator::Invalid) {
			return QValidator::Invalid;
		}

		return !(input.toInt() % (1 << (ADDR_SHIFT + 1))) ? QValidator::Acceptable : QValidator::Intermediate;
	}
};
}

AdpcmStartAddressSpinBox::AdpcmStartAddressSpinBox(QWidget* parent)
	: QSpinBox(parent)
{
	setSingleStep(1 << (ADDR_SHIFT + 1));
	setMinimum(start::addressToPosition(0));
	setValue(minimum());
	setMaximum(minimum());
}

int AdpcmStartAddressSpinBox::valueByAddress() const
{
	return start::positionToAddress(value());
}

void AdpcmStartAddressSpinBox::setValueByAddress(int a)
{
	setValue(start::addressToPosition(a));
}

void AdpcmStartAddressSpinBox::setValueByBytes(int b)
{
	setValue(start::bytesToPosition(b));
}

void AdpcmStartAddressSpinBox::setMaximumByBytes(int b)
{
	setMaximum(start::bytesToPosition(b));
}

QValidator::State AdpcmStartAddressSpinBox::validate(QString& text, int& pos) const
{
	return StartAddressValidator(minimum(), maximum()).validate(text, pos);
}

namespace
{
namespace stop
{
constexpr int addressToPosition(int a)
{
	// (((a << ADDR_SHIFT) & 31) << 1) + 1
	return (a << (ADDR_SHIFT + 1)) + 63;
}

constexpr int bytesToPosition(int b)
{
	return addressToPosition(b >> ADDR_SHIFT);
}

constexpr int positionToAddress(int p)
{
	return p >> (ADDR_SHIFT + 1);
}
}

class StopAddressValidator : public QIntValidator
{
public:
	StopAddressValidator(int bottom, int top, QObject *parent = nullptr) : QIntValidator(bottom, top, parent) {}
	QValidator::State validate(QString& input, int& pos) const override
	{
		auto state = QIntValidator::validate(input, pos);
		if (state == QIntValidator::Invalid) {
			return QValidator::Invalid;
		}

		return !((input.toInt() + 1) % (1 << (ADDR_SHIFT + 1))) ? QValidator::Acceptable : QValidator::Intermediate;
	}
};
}

AdpcmStopAddressSpinBox::AdpcmStopAddressSpinBox(QWidget* parent)
	: QSpinBox(parent)
{
	setSingleStep(1 << (ADDR_SHIFT + 1));
	setMinimum(stop::addressToPosition(0));
	setValue(minimum());
	setMaximum(minimum());
}

int AdpcmStopAddressSpinBox::valueByAddress() const
{
	return stop::positionToAddress(value());
}

void AdpcmStopAddressSpinBox::setValueByAddress(int a)
{
	setValue(stop::addressToPosition(a));
}

void AdpcmStopAddressSpinBox::setValueByBytes(int b)
{
	setValue(stop::bytesToPosition(b));
}

void AdpcmStopAddressSpinBox::setMaximumByBytes(int b)
{
	setMaximum(stop::bytesToPosition(b));
}

QValidator::State AdpcmStopAddressSpinBox::validate(QString& text, int& pos) const
{
	return StopAddressValidator(minimum(), maximum()).validate(text, pos);
}
