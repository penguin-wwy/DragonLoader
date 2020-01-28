//
// Created by penguin on 1/27/20.
//

#include <iostream>
#include <stdlib.h>

#include "loader.h"

using AddOpFunc = int (*)(int, int);

int main(int argc, char **argv) {
	if (argc < 5) {
		std::cout << "Error input." << std::endl;
		std::cout << "Usage: IE <num> <num> <source file path> -I<include file path>" << std::endl;
		return 0;
	}
	DragonLoader loader;
	std::string errInfo;
	std::vector<const char *> argList = { argv[4] };
	loader.loadSourceFile(argv[3], argList, errInfo);
	auto func = reinterpret_cast<AddOpFunc>(loader.getNamedFunction("addOpFunc(int, int)"));
	if (func == nullptr) {
		std::cout << "Get function address failed." << std::endl;
		return 0;
	}
	int i = atoi(argv[1]);
	int j = atoi(argv[2]);
	int result = func(i, j);
	std::cout << "number: " << argv[1] << " " << argv[2] << "\nresult: " << result << std::endl;
	return 0;
}