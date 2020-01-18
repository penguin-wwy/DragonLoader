//
// Created by penguin on 20-1-15.
//

#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/ExecutionEngine/MCJIT.h"

#include "loader.h"

using namespace llvm;

DragonLoader::DragonLoader() : mm(new SectionMemoryManager), mArch("") {
	InitializeNativeTarget();
	InitializeNativeTargetAsmParser();
	InitializeNativeTargetAsmPrinter();
}

void DragonLoader::close() {
	ee->runStaticConstructorsDestructors(true);
}

DragonLoader* DragonLoader::loadBitcodeFile(const char *filePath, raw_ostream& os) {
	SMDiagnostic parseErr;
	std::unique_ptr<llvm::Module> module = parseIRFile(filePath, parseErr, this->context);
	if (module == nullptr) {
		parseErr.print("dragon loader", os);
		return nullptr;
	}
	std::string buildErr;
	ee = EngineBuilder (std::move(module)).setErrorStr(&buildErr).setEngineKind(EngineKind::JIT).create();
	if (ee == nullptr) {
		os << buildErr;
		return nullptr;
	}
	ee->finalizeObject();
	ee->runStaticConstructorsDestructors(false);
	return this;
}

DragonLoader* DragonLoader::registeMethod(const char *methodName) {
	// build cache
}

void *DragonLoader::getNamedFunction(const char *name) {
	void *address = ee->getPointerToNamedFunction(name);
	return address;
}
