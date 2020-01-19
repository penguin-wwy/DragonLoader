//
// Created by penguin on 20-1-15.
//

#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/Program.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/ExecutionEngine/MCJIT.h"

#include "clang/Basic/Diagnostic.h"
#include "clang/CodeGen/CodeGenAction.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/TextDiagnosticBuffer.h"
#include "clang/Driver/Driver.h"

#include "loader.h"

using namespace llvm;
using namespace clang;

DragonLoader::DragonLoader() : mm(new SectionMemoryManager), mArch("") {
	InitializeNativeTarget();
	InitializeNativeTargetAsmParser();
	InitializeNativeTargetAsmPrinter();
}

void DragonLoader::close() {
	ee->runStaticConstructorsDestructors(true);
}

bool DragonLoader::createExecutionEngin(std::unique_ptr<llvm::Module> module, raw_ostream& os) {
	std::string buildErr;
	ee = EngineBuilder (std::move(module)).setErrorStr(&buildErr).setEngineKind(EngineKind::JIT).create();
	if (ee == nullptr) {
		os << buildErr;
		return false;
	}
	ee->finalizeObject();
	ee->runStaticConstructorsDestructors(false);
	return true;
}

DragonLoader* DragonLoader::loadSourceFile(const char *filePath, raw_ostream& os) {
	std::vector<const char *> args;

	ErrorOr<std::string> clangPath = sys::findProgramByName("clang");
	if (!clangPath) {
		os << clangPath.getError().message();
		return nullptr;
	}
	args.push_back((*clangPath).c_str());
	ArrayRef<const char *> argList(args);

	IntrusiveRefCntPtr<DiagnosticIDs> diagID(new DiagnosticIDs());
	IntrusiveRefCntPtr<DiagnosticOptions> diagOpt = new DiagnosticOptions();
	auto *diagBuffer = new TextDiagnosticBuffer();
	DiagnosticsEngine diags(diagID, &*diagOpt, diagBuffer);
	std::unique_ptr<CompilerInstance> clang(new CompilerInstance());
	CompilerInvocation::CreateFromArgs(clang->getInvocation(), argList.begin(), argList.end(), diags);

	clang->createDiagnostics();
	if (!clang->hasDiagnostics()) {
		os << "create diagnostics error.";
	}
	std::unique_ptr<CodeGenAction> action(make_unique<EmitBCAction>());
	if (!clang->ExecuteAction(*action)) {
		os << "emit bc error.";
	}
	return createExecutionEngin(std::move(action->takeModule()), os) ? this : nullptr;
}

DragonLoader* DragonLoader::loadBitcodeFile(const char *filePath, raw_ostream& os) {
	SMDiagnostic parseErr;
	std::unique_ptr<llvm::Module> module = parseIRFile(filePath, parseErr, this->context);
	if (module == nullptr) {
		parseErr.print("dragon loader", os);
		return nullptr;
	}
	return createExecutionEngin(std::move(module), os) ? this : nullptr;
}

DragonLoader* DragonLoader::registeMethod(const char *methodName) {
	// build cache
}

void *DragonLoader::getNamedFunction(const char *name) {
	void *address = ee->getPointerToNamedFunction(name);
	return address;
}
