#include "bank_io.hpp"
#include <nowide/fstream.hpp>
#include "file_io.hpp"
#include "file_io_error.hpp"

BankIO::BankIO()
{
}

AbstractBank* BankIO::loadBank(std::string path)
{
	std::string ext = path.substr(path.find_last_of(".")+1);
	if (ext.compare("wopn") == 0) return BankIO::loadWOPNFile(path);
	throw FileInputError(FileIO::FileType::BANK);
}

AbstractBank* BankIO::loadWOPNFile(std::string path)
{
	struct WOPNDeleter {
		void operator()(WOPNFile *x) { WOPN_Free(x); }
	};

	std::unique_ptr<WOPNFile, WOPNDeleter> wopn;

	nowide::ifstream in(path, std::ios::binary);
	in.seekg(0, std::ios::end);
	std::streampos size = in.tellg();

	if (!in)
		throw FileInputError(FileIO::FileType::BANK);
	else {
		std::unique_ptr<char[]> buf(new char[size]);
		in.seekg(0, std::ios::beg);
		if (!in.read(buf.get(), size) || in.gcount() != size)
			throw FileInputError(FileIO::FileType::BANK);
		wopn.reset(WOPN_LoadBankFromMem(buf.get(), size, nullptr));
		if (!wopn)
			throw FileCorruptionError(FileIO::FileType::BANK);
	}

	WopnBank *bank = new WopnBank(wopn.get());
	wopn.release();
	return bank;
}
