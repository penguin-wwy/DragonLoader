//
// Created by penguin on 20-1-15.
//

#include <iostream>
#include <fstream>

#include "llvm/Support/FileSystem.h"
#include "gtest/gtest.h"

#include "loader.h"

TEST(Hello, Test1) {
	const char *code = "; ModuleID = 'AddFunc.bc'\n"
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
	const char *fileName = "/tmp/AddFunc.ll";
	std::ofstream tmpFile;
	tmpFile.open(fileName);
	tmpFile << code;
	tmpFile.close();
	DragonLoader loader;
	loader.loadBitcodeFile(fileName);
	int (*addFunc)(int, int) = (int(*)(int, int))loader.getNamedFunction("addFunc");
	EXPECT_EQ(3, addFunc(1, 2));
}