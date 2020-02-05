//
// Created by penguin on 20-1-15.
//

#ifndef DRAGONLOADER_LOADER_H
#define DRAGONLOADER_LOADER_H

#include <bits/unique_ptr.h>
#include <unordered_map>
#include <vector>

namespace llvm {
	class LLVMContext;
	class Module;
	class Function;
	class ExecutionEngine;
	class raw_ostream;
}
class CompilerEngin;
class DragonLoader {
public:
	enum EmitType {
		BC,
		LL,
		OBJ
	};

private:
	llvm::LLVMContext *context = nullptr;
	llvm::ExecutionEngine *ee = nullptr;
	CompilerEngin *compiler = nullptr;

	std::vector<llvm::Function *> functions;
	std::unordered_map<std::string, void *> address;

public:
	DragonLoader();

	void close();

	void resetCompiler();

	void finalizeLoad();

	DragonLoader *loadSourceFile(const char *, std::string &);

	DragonLoader *loadSourceFile(const char *, std::vector<const char *> &, std::string &);

	DragonLoader *loadBitcodeFile(const char *, std::string &);

	DragonLoader *appendSharedLibrary(const char *, std::string &);

	DragonLoader *appendExtraArchive(const char *archive, std::string &err);

//	DragonLoader *registeMethod(const char *);

	void *getNamedFunction(const char *);

	void *getNamedCFunction(const char *);

	static bool compileSource(DragonLoader *, EmitType, const char *, std::vector<const char *> &, std::string &);

private:
	bool createExecutionEngin(std::unique_ptr<llvm::Module>, llvm::raw_ostream &);
};

#endif //DRAGONLOADER_LOADER_H
