//
// Created by penguin on 20-1-15.
//

#ifndef DRAGONLOADER_LOADER_H
#define DRAGONLOADER_LOADER_H

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/ExecutionEngine/RTDyldMemoryManager.h"

class DragonLoader {
private:
	llvm::LLVMContext context;
	std::unique_ptr<llvm::RTDyldMemoryManager> mm;
	llvm::ExecutionEngine *ee = nullptr;
	llvm::StringRef mArch;

public:
	DragonLoader();

	void close();

	DragonLoader *loadSourceFile(const char *, llvm::raw_ostream&);

	DragonLoader *loadBitcodeFile(const char *, llvm::raw_ostream&);

	DragonLoader *registeMethod(const char *);

	void *getNamedFunction(const char *);

private:
	bool createExecutionEngin(std::unique_ptr<llvm::Module>, llvm::raw_ostream &);
};

#endif //DRAGONLOADER_LOADER_H
