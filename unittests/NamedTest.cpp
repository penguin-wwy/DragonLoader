//
// Created by penguin on 1/27/20.
//

#include "llvm/Demangle/Demangle.h"

#include "gtest/gtest.h"

TEST(LLVM_Mangle, Test1) {
	std::string name = llvm::demangle("_Z3addii");
	EXPECT_TRUE(name == "add(int, int)") << "Mangle name error: " << name;

	name = llvm::demangle("sub");
	EXPECT_TRUE(name == "sub") << "Mangle name error: " << name;
}