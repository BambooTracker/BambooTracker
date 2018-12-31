#pragma once

#include <string>
#include "bank.hpp"
#include "format/wopn_file.h"

class BankIO
{
public:
	static AbstractBank* loadBank(std::string path);
	static AbstractBank* loadWOPNFile(std::string path);

private:
	BankIO();
};
