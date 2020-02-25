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
	static AbstractBank* loadPPCFile(BinaryContainer& ctr);
	static AbstractBank* loadPVIFile(BinaryContainer& ctr);

private:
	BankIO();

	static void extractADPCMSamples(const BinaryContainer& ctr, size_t addrPos, size_t sampOffs, int maxCnt,
									std::vector<int>& ids, std::vector<std::vector<uint8_t>>& samples);
};
