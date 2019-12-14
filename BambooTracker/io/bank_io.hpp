#pragma once

#include <string>
#include <memory>
#include <vector>
#include "bank.hpp"
#include "module.hpp"
#include "binary_container.hpp"
#include "format/wopn_file.h"

class BankIO
{
public:
	static void saveBank(BinaryContainer& ctr, std::vector<int> instNums,
						 std::weak_ptr<InstrumentsManager> instMan);
	static AbstractBank* loadBank(BinaryContainer& ctr, std::string path);
	static AbstractBank* loadBTBFile(BinaryContainer& ctr);
	static AbstractBank* loadWOPNFile(BinaryContainer& ctr);

private:
	BankIO();
};
