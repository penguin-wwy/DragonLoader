//
// Created by penguin on 20-1-15.
//

#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/IRReader/IRReader.h"

#include "loader.h"

using namespace llvm;

DragonLoader::DragonLoader() : mm(new SectionMemoryManager), mArch("") {
	InitializeNativeTarget();
	InitializeNativeTargetAsmParser();
	InitializeNativeTargetAsmPrinter();
}

DragonLoader* DragonLoader::loadBitcodeFile(const char *filePath) {
	SMDiagnostic err;
	module = parseIRFile(filePath, err, this->context);
	if (module == nullptr) {
		err.print("dragon loader", errs());
		return nullptr;
	}
	return this;
}

uint64_t DragonLoader::getNamedFunction(const char *name) {
	std::string errMsg;
	ee = EngineBuilder (std::move(module))
			.setMArch("")
			.setMCPU(sys::getHostCPUName())
			.setMCJITMemoryManager(std::move(mm))
			.setErrorStr(&errMsg).create();
	if (ee == nullptr) {
		errs() << errMsg << "\n";
		return 0;
	}
	uint64_t address = ee->getFunctionAddress(name);
	return address;
}
