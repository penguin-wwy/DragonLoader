//
// Created by penguin on 20-1-15.
//

#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/Demangle/Demangle.h"
#include "llvm/Object/ObjectFile.h"
#include "llvm/Object/Archive.h"

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
		std::unique_ptr<CodeGenAction> action = createCodeGenAction(DragonLoader::LL, context);
		if (!compiler->ExecuteAction(*action)) {
			os << "emit bc error.";
			return nullptr;
		}
		return std::move(action->takeModule());
	}

	bool compile(ArrayRef<const char *> &args, DragonLoader::EmitType type, LLVMContext *context, raw_ostream &os) {
		CompilerInvocation::CreateFromArgs(compiler->getInvocation(), args.begin(), args.end(), diagEngine);
		compiler->createDiagnostics();
		if (!compiler->hasDiagnostics()) {
			os << "create diagnostics error.";
			return false;
		}
		std::unique_ptr<CodeGenAction> action = createCodeGenAction(type, context);
		return compiler->ExecuteAction(*action);
	}

	static std::unique_ptr<CodeGenAction> createCodeGenAction(DragonLoader::EmitType type, LLVMContext *context) {
		switch (type) {
			case DragonLoader::BC:
				return make_unique<EmitBCAction>(context);
			case DragonLoader::LL:
				return make_unique<EmitLLVMOnlyAction>(context);
			case DragonLoader::OBJ:
				return make_unique<EmitObjAction>(context);
		}
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
	std::string buildErr;
	ee = EngineBuilder (std::move(module)).setErrorStr(&buildErr).setEngineKind(EngineKind::JIT).create();
	if (ee == nullptr) {
		os << buildErr;
		return false;
	}
	return true;
}

void DragonLoader::finalizeLoad() {
	if (ee == nullptr) {
		return;
	}
	for (Function *func : functions) {
		std::string name = demangle(func->getName().str());
		name.erase(std::remove(name.begin(), name.end(), ' '), name.end());
		address[name] = ee->getPointerToFunction(func);
	}
	ee->finalizeObject();
	ee->runStaticConstructorsDestructors(false);
}

DragonLoader* DragonLoader::loadSourceFile(const char *filePath, std::string &err) {
	std::vector<const char *> argv = { filePath };
	return loadSourceFile(nullptr, argv, err);
}

DragonLoader* DragonLoader::loadSourceFile(const char *filePath, std::vector<const char *> &argv, std::string &err) {
	raw_string_ostream os(err);
	if (compiler == nullptr) {
		compiler = new CompilerEngin();
	}

	if (filePath != nullptr) {
		argv.emplace_back(filePath);
	}

	ArrayRef<const char *> argList(argv);
	std::unique_ptr<llvm::Module> module = compiler->compileModule(argList, context, os);
	if (module == nullptr) {
		return nullptr;
	}
	for (auto &func : module->functions()) {
		if (func.isDeclaration()) {
			functions.emplace_back(&func);
		}
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
	for (auto &func : module->functions()) {
		if (func.isDeclaration()) {
			functions.emplace_back(&func);
		}
	}
	return createExecutionEngin(std::move(module), os) ? this : nullptr;
}

DragonLoader *DragonLoader::appendSharedLibrary(const char *sharedLibrary, std::string &err) {
	raw_string_ostream os(err);
	if (ee == nullptr) {
		os << "Can't find loaded source file or bitcode file.";
		return nullptr;
	}

	Expected<object::OwningBinary<object::ObjectFile>> obj = object::ObjectFile::createObjectFile(sharedLibrary);
	if (!obj) {
		os << "Get shared library " << sharedLibrary << "failed.";
		return nullptr;
	}

	object::OwningBinary<object::ObjectFile> &file = obj.get();
	ee->addObjectFile(std::move(file));
	return this;
}

DragonLoader* DragonLoader::appendExtraArchive(const char *archive, std::string &err) {
	raw_string_ostream os(err);
	if (ee == nullptr) {
		os << "Can't find loaded source file or bitcode file.";
		return nullptr;
	}

	ErrorOr<std::unique_ptr<MemoryBuffer>> archBufOrErr = MemoryBuffer::getFileOrSTDIN(archive);
	if (!archBufOrErr) {
		os << "Get extra archive error code: " << archBufOrErr.getError().value();
		return nullptr;
	}

	Expected<std::unique_ptr<object::Archive>> archOrErr = object::Archive::create(archBufOrErr.get()->getMemBufferRef());
	if (!archOrErr) {
		os << archOrErr.takeError();
		return nullptr;
	}

	object::OwningBinary<object::Archive> oa(std::move(archOrErr.get()), std::move(archBufOrErr.get()));
	ee->addArchive(std::move(oa));
	return this;
}

void *DragonLoader::getNamedFunction(const char *name) {
	std::string funcName(name);
	funcName.erase(std::remove(funcName.begin(), funcName.end(), ' '), funcName.end());
	return address[funcName];
}

void *DragonLoader::getNamedCFunction(const char *name) {
	return address[name];
}

bool DragonLoader::compileSource(DragonLoader *dragonLoader, DragonLoader::EmitType emitType, const char *filePath,
		std::vector<const char *> &argv, std::string &err) {
	raw_string_ostream os(err);
	if (dragonLoader->compiler == nullptr) {
		dragonLoader->compiler = new CompilerEngin();
	}

	if (filePath != nullptr) {
		argv.emplace_back(filePath);
	}

	ArrayRef<const char *> argList(argv);
	return dragonLoader->compiler->compile(argList, emitType, nullptr, os);
}

void DragonLoader::resetCompiler() {
	delete compiler;
	compiler = new CompilerEngin();
}
