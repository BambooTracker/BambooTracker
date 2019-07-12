#pragma once

#include <string>
#include <memory>
#include <vector>
#include "bank.hpp"
#include "module.hpp"
#include "format/wopn_file.h"

class BankIO
{
public:
	static void saveBank(std::string path, std::vector<size_t> instNums,
						 std::weak_ptr<InstrumentsManager> instMan);
	static AbstractBank* loadBank(std::string path);
	static AbstractBank* loadWOPNFile(std::string path);

private:
	BankIO();
};
