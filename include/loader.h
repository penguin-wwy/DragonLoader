//
// Created by penguin on 20-1-15.
//

#ifndef DRAGONLOADER_LOADER_H
#define DRAGONLOADER_LOADER_H

#include <bits/unique_ptr.h>
#include <unordered_map>

namespace llvm {
	class LLVMContext;
	class Module;
	class ExecutionEngine;
	class raw_ostream;
}
class CompilerEngin;
class DragonLoader {
private:
	llvm::LLVMContext *context = nullptr;
	llvm::ExecutionEngine *ee = nullptr;
	CompilerEngin *compiler = nullptr;

	std::unordered_map<std::string, void *> address;

public:
	DragonLoader();

	void close();

	DragonLoader *loadSourceFile(const char *, std::string &);

	DragonLoader *loadBitcodeFile(const char *, std::string &);

	DragonLoader *registeMethod(const char *);

	void *getNamedFunction(const char *);

private:
	bool createExecutionEngin(std::unique_ptr<llvm::Module>, llvm::raw_ostream &);
};

#endif //DRAGONLOADER_LOADER_H
