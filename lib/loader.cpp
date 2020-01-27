//
// Created by penguin on 20-1-15.
//

#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/Program.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/Demangle/Demangle.h"

#include "clang/Basic/Diagnostic.h"
#include "clang/CodeGen/CodeGenAction.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/TextDiagnosticBuffer.h"

#include "loader.h"

using namespace llvm;
using namespace clang;

struct CompilerEngin {
	std::unique_ptr<CompilerInstance> compiler;
	IntrusiveRefCntPtr<DiagnosticIDs> diagID;
	IntrusiveRefCntPtr<DiagnosticOptions> diagOpt;
	TextDiagnosticBuffer *diagBuffer;
	DiagnosticsEngine diagEngine;

	CompilerEngin() : compiler(new CompilerInstance()), diagID(new DiagnosticIDs()),
					  diagOpt(new DiagnosticOptions()), diagBuffer(new TextDiagnosticBuffer()),
					  diagEngine(diagID, &*diagOpt, diagBuffer) {
	}

	std::unique_ptr<llvm::Module> compileModule(ArrayRef<const char *> &args, LLVMContext *context, raw_ostream &os) {
		CompilerInvocation::CreateFromArgs(compiler->getInvocation(), args.begin(), args.end(), diagEngine);
		compiler->createDiagnostics();
		if (!compiler->hasDiagnostics()) {
			os << "create diagnostics error.";
		}
		std::unique_ptr<CodeGenAction> action(make_unique<EmitLLVMOnlyAction>(context));
		if (!compiler->ExecuteAction(*action)) {
			os << "emit bc error.";
			return nullptr;
		}
		return std::move(action->takeModule());
	}
};

DragonLoader::DragonLoader() : context(new LLVMContext()) {
	InitializeNativeTarget();
	InitializeNativeTargetAsmParser();
	InitializeNativeTargetAsmPrinter();
}

void DragonLoader::close() {
	ee->runStaticConstructorsDestructors(true);
}

bool DragonLoader::createExecutionEngin(std::unique_ptr<llvm::Module> module, raw_ostream& os) {
	std::vector<Function *> funcs;
	for (auto &func : module->functions()) {
		funcs.push_back(&func);
	}

	std::string buildErr;
	ee = EngineBuilder (std::move(module)).setErrorStr(&buildErr).setEngineKind(EngineKind::JIT).create();
	if (ee == nullptr) {
		os << buildErr;
		return false;
	}
	for (Function *func : funcs) {
		std::string name = demangle(func->getName().str());
		name.erase(std::remove(name.begin(), name.end(), ' '), name.end());
		address[name] = ee->getPointerToFunction(func);
	}
	ee->finalizeObject();
	ee->runStaticConstructorsDestructors(false);
	return true;
}

DragonLoader* DragonLoader::loadSourceFile(const char *filePath, std::string &err) {
	raw_string_ostream os(err);
	if (compiler == nullptr) {
		compiler = new CompilerEngin();
	}

	std::vector<const char *> args;
	args.push_back(filePath);
	ArrayRef<const char *> argList(args);
	std::unique_ptr<llvm::Module> module = compiler->compileModule(argList, context, os);
	if (module == nullptr) {
		return nullptr;
	}
	return createExecutionEngin(std::move(module), os) ? this : nullptr;
}

DragonLoader* DragonLoader::loadBitcodeFile(const char *filePath, std::string &err) {
	raw_string_ostream os(err);
	SMDiagnostic parseErr;
	std::unique_ptr<llvm::Module> module = parseIRFile(filePath, parseErr, *context);
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
	std::string funcName(name);
	funcName.erase(std::remove(funcName.begin(), funcName.end(), ' '), funcName.end());
	return address[funcName];
}

void *DragonLoader::getNamedCFunction(const char *name) {
	return address[name];
}
