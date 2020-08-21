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
	static void saveBank(BinaryContainer& ctr, const std::vector<int>& instNums,
						 const std::weak_ptr<InstrumentsManager> instMan);
	static AbstractBank* loadBank(const BinaryContainer& ctr, const std::string& path);
	static AbstractBank* loadBTBFile(const BinaryContainer& ctr);
	static AbstractBank* loadWOPNFile(const BinaryContainer& ctr);
	static AbstractBank* loadFFFile(const BinaryContainer& ctr);
	static AbstractBank* loadPPCFile(const BinaryContainer& ctr);
	static AbstractBank* loadPVIFile(const BinaryContainer& ctr);
	static AbstractBank* loadMucom88File(const BinaryContainer& ctr);

private:
	BankIO();

	static void extractADPCMSamples(const BinaryContainer& ctr, size_t addrPos, size_t sampOffs, int maxCnt,
									std::vector<int>& ids, std::vector<std::vector<uint8_t>>& samples);
};
