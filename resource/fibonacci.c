//
// Created by penguin on 20-1-21.
//

int fibfunction(int number) {
	if (number <= 0) {
		return 0;
	} else if (number == 1 || number == 2) {
		return 1;
	} else if (number > 100) {
		number = 100;
	}
	return fibfunction(number - 1) + fibfunction(number - 2);
}