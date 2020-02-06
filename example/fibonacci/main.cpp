//
// Created by penguin on 20-1-21.
//

#include <iostream>
#include <stdlib.h>

#include "loader.h"

using fibonacci = int (*)(int);

int main(int argc, char **argv) {
	if (argc < 3) {
		std::cout << "Error input." << std::endl;
		std::cout << "Usage: FF <num> <source file path>" << std::endl;
		return 0;
	}
	DragonLoader loader;
	std::string errInfo;
	loader.loadSourceFile(argv[2], errInfo);
	loader.finalizeLoad();
	auto func = reinterpret_cast<fibonacci>(loader.getNamedCFunction("fibfunction"));
	if (func == nullptr) {
		std::cout << "Get function address failed." << std::endl;
		return 0;
	}
	int i = atoi(argv[1]);
	int result = func(i);
	std::cout << "number: " << argv[1] << "\nresult: " << result << std::endl;
	return 0;
}