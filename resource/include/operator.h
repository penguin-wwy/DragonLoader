//
// Created by penguin on 1/27/20.
//

#ifndef DRAGONLOADER_OPERATOR_H
#define DRAGONLOADER_OPERATOR_H

enum op {
	Add = 0,
	Sub,
	Mul,
	Div
};

int opFunc(op opcode, int a, int b) {
	int res = 0;
	switch (opcode) {
		case Add: {
			res = a + b;
			break;
		}
		case Sub: {
			res = a - b;
			break;
		}
		case Mul: {
			res = a * b;
			break;
		}
		case Div: {
			res = a / b;
			break;
		}
		default:
			break;
	}
	return res;
}

#endif //DRAGONLOADER_EXAMPLE_H
