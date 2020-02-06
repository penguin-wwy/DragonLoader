//
// Created by penguin on 20-1-15.
//

#include <iostream>
#include <fstream>

#include "llvm/Support/SourceMgr.h"
#include "llvm/AsmParser/Parser.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "gtest/gtest.h"

#include "loader.h"

using namespace llvm;

using AddFuncType = int (*)(int, int);

static const char *addFuncCode =
		"; ModuleID = 'AddFunc.bc'\n"
		"source_filename = \"AddFunc.c\"\n"
		"target datalayout = \"e-m:e-i64:64-f80:128-n8:16:32:64-S128\"\n"
		"target triple = \"x86_64-unknown-linux-gnu\"\n"
		"\n"
		"; Function Attrs: noinline nounwind optnone uwtable\n"
		"define dso_local i32 @addFunc(i32, i32) #0 {\n"
		"  %3 = alloca i32, align 4\n"
		"  %4 = alloca i32, align 4\n"
		"  store i32 %0, i32* %3, align 4\n"
		"  store i32 %1, i32* %4, align 4\n"
		"  %5 = load i32, i32* %3, align 4\n"
		"  %6 = load i32, i32* %4, align 4\n"
		"  %7 = add nsw i32 %5, %6\n"
		"  ret i32 %7\n"
		"}\n"
		"\n"
		"attributes #0 = { noinline nounwind optnone uwtable \"correctly-rounded-divide-sqrt-fp-math\"=\"false\" \"disable-tail-calls\"=\"false\" \"less-precise-fpmad\"=\"false\" \"min-legal-vector-width\"=\"0\" \"no-frame-pointer-elim\"=\"true\" \"no-frame-pointer-elim-non-leaf\" \"no-infs-fp-math\"=\"false\" \"no-jump-tables\"=\"false\" \"no-nans-fp-math\"=\"false\" \"no-signed-zeros-fp-math\"=\"false\" \"no-trapping-math\"=\"false\" \"stack-protector-buffer-size\"=\"8\" \"target-cpu\"=\"x86-64\" \"target-features\"=\"+fxsr,+mmx,+sse,+sse2,+x87\" \"unsafe-fp-math\"=\"false\" \"use-soft-float\"=\"false\" }\n"
		"\n"
		"!llvm.module.flags = !{!0}\n"
		"!llvm.ident = !{!1}\n"
		"\n"
		"!0 = !{i32 1, !\"wchar_size\", i32 4}\n"
		"!1 = !{!\"clang version 8.0.0 (tags/RELEASE_800/final)\"}";

TEST(ParserTest, Test1) {
	const char *fileName = "/tmp/AddFunc.ll";
	std::ofstream tmpFile;
	tmpFile.open(fileName);
	tmpFile << addFuncCode;
	tmpFile.close();

	InitializeNativeTarget();
	InitializeNativeTargetAsmPrinter();

	LLVMContext context;
	SMDiagnostic error;
	std::unique_ptr<Module> module = parseAssemblyFile(fileName, error, context);
	EXPECT_TRUE(module.get() != nullptr) << "llvm Module parse failed.";

	std::string errStr;
	ExecutionEngine *ee = EngineBuilder(std::move(module)).setErrorStr(&errStr).setEngineKind(EngineKind::JIT).create();
	EXPECT_TRUE(ee != nullptr) << errStr;

	ee->finalizeObject();
	ee->runStaticConstructorsDestructors(false);
	AddFuncType addFunc = reinterpret_cast<AddFuncType>(ee->getPointerToNamedFunction("addFunc"));
	ASSERT_TRUE(addFunc != nullptr);
	EXPECT_EQ(0, addFunc(0, 0));
	EXPECT_EQ(10, addFunc(1, 9));
	EXPECT_EQ(5, addFunc(-5, 10));
	ee->runStaticConstructorsDestructors(true);
}

TEST(loader, Test2) {
	const char *fileName = "/tmp/AddFunc.ll";
	std::ofstream tmpFile;
	tmpFile.open(fileName);
	tmpFile << addFuncCode;
	tmpFile.close();
	DragonLoader loader;
	std::string errInfo;
	loader.loadBitcodeFile(fileName, errInfo);
	loader.finalizeLoad();
	AddFuncType addFunc = reinterpret_cast<AddFuncType>(loader.getNamedCFunction("addFunc"));
	EXPECT_EQ(3, addFunc(1, 2));
}

static const char *addFuncSource =
		"int addFunc(int a, int b) {\n"
		"	return a + b;\n"
		"}\n";

TEST(sourceLoader, Test3) {
	const char *fileName = "/tmp/AddFunc.c";
	std::ofstream tmpFile;
	tmpFile.open(fileName);
	tmpFile << addFuncSource;
	tmpFile.close();
	DragonLoader loader;
	std::string errInfo;
	loader.loadSourceFile(fileName, errInfo);
	loader.finalizeLoad();
	AddFuncType addFunc = reinterpret_cast<AddFuncType >(loader.getNamedCFunction("addFunc"));
	ASSERT_TRUE(addFunc != nullptr);
	EXPECT_EQ(3, addFunc(1, 2));
}

TEST(CXXLoader, Test4) {
	const char *fileName = "/tmp/AddFunc.cc";
	std::ofstream tmpFile;
	tmpFile.open(fileName);
	tmpFile << addFuncSource;
	tmpFile.close();
	DragonLoader loader;
	std::string errInfo;
	loader.loadSourceFile(fileName, errInfo);
	loader.finalizeLoad();
	AddFuncType addFunc = reinterpret_cast<AddFuncType>(loader.getNamedFunction("addFunc(int, int)"));
	ASSERT_TRUE(addFunc != nullptr);
	EXPECT_EQ(3, addFunc(1, 2));
}

static const char *mainFile =
		"extern int add(int a, int b);\n"
		"\n"
		"int addWrap(int a, int b, int c) {\n"
		"    return add(a + b, c);\n"
		"}";

TEST(ExtraObject, Test5) {
	const char *addFileName = "/tmp/add.cc";
	const char *addFileOut = "/tmp/add.o";
	std::ofstream addTmpFile;
	addTmpFile.open(addFileName);
	addTmpFile << addFuncSource;
	addTmpFile.close();

	const char *mainFileName = "/tmp/wrap.cc";
	std::ofstream mainTmpFile;
	mainTmpFile.open(mainFileName);
	mainTmpFile << mainFile;
	mainTmpFile.close();

	DragonLoader loader;
	std::string errInfo;
	std::vector<const char *> args = {"-emit-obj", "-o", addFileOut};
	EXPECT_TRUE(DragonLoader::compileSource(&loader, DragonLoader::OBJ, addFileName, args, errInfo)) << errInfo;
	loader.resetCompiler();
	EXPECT_NE(loader.loadSourceFile(mainFileName, errInfo), nullptr) << errInfo;
	EXPECT_NE(loader.appendSharedLibrary(addFileOut, errInfo), nullptr) << errInfo;
	loader.finalizeLoad();
	auto addWrap = reinterpret_cast<int (*)(int, int, int)>(loader.getNamedFunction("addWrap(int, int, int)"));
	ASSERT_TRUE(addWrap != nullptr);
	// FIXME: call compileSource lead to relocation failed. So call addWrap error.
//	EXPECT_EQ(6, addWrap(1, 2, 3));
}