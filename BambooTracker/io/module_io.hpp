#pragma once

#include <memory>
#include <string>
#include "module.hpp"
#include "instruments_manager.hpp"
#include "binary_container.hpp"

class ModuleIO
{
public:
	static void saveModule(std::string path, std::weak_ptr<Module> mod,
						   std::weak_ptr<InstrumentsManager> instMan);
	static void loadModule(std::string path, std::weak_ptr<Module> mod,
						   std::weak_ptr<InstrumentsManager> instMan);
	static void backupModule(std::string path);

private:
	ModuleIO();

	static size_t loadModuleSectionInModule(std::weak_ptr<Module> mod, BinaryContainer& ctr,
											size_t globCsr, uint32_t version);
	static size_t loadInstrumentSectionInModule(std::weak_ptr<InstrumentsManager> instMan,
												BinaryContainer& ctr, size_t globCsr, uint32_t version);
	static size_t loadInstrumentPropertySectionInModule(std::weak_ptr<InstrumentsManager> instMan,
														BinaryContainer& ctr, size_t globCsr,
														uint32_t version);
	static size_t loadInstrumentPropertyOperatorSequence(FMEnvelopeParameter param,
														 size_t instMemCsr,
														 std::weak_ptr<InstrumentsManager> instMan,
														 BinaryContainer& ctr, uint32_t version);
	static size_t loadGrooveSectionInModule(std::weak_ptr<Module> mod, BinaryContainer& ctr,
											size_t globCsr, uint32_t version);
	static size_t loadSongSectionInModule(std::weak_ptr<Module> mod, BinaryContainer& ctr,
										  size_t globCsr, uint32_t version);
};
