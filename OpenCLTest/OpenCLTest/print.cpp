#include "matrix.h"

static void printFloat(float x) {
	printf("|%-13f|", x);
}

template <unsigned int R, unsigned int C>
void Matrix<R, C>::print(){
	std::cout << "Matrix [" << R << " x " << C << "]\n";
	if (C <= 6) {
		if (R <= 20) {
			for (int row = 0; row < R; row++) {
				for (int col = 0; col < C; col++) {
					printFloat(matrix[row * C + col]);
				}
				std::cout << "\n";
			}
		}
		else {
			for (int row = 0; row < 10; row++) {
				for (int col = 0; col < C; col++) {
					printFloat(matrix[row * C + col]);
				}
				std::cout << "\n";
			}
			for (int col = 0; col < C; col++) {
				std::cout << "  ....       ";
			}
			std::cout << "\n";
			for (int row = R - 10; row < R; row++) {
				for (int col = 0; col < C; col++) {
					printFloat(matrix[row * C + col]);
				}
				std::cout << "\n";
			}
		}
	}
	else {
		if (R <= 20) {
			for (int row = 0; row < R; row++) {
				for (int col = 0; col < 3; col++) {
					printFloat(matrix[row * C + col]);
				}
				std::cout << "  ....       ";
				for (int col = C - 2; col < C; col++) {
					printFloat(matrix[row * C + col]);
				}
				std::cout << "\n";
			}
		}
		else {
			for (int row = 0; row < 10; row++) {
				for (int col = 0; col < 3; col++) {
					printFloat(matrix[row * C + col]);
				}
				std::cout << "  ....       ";
				for (int col = C - 2; col < C; col++) {
					printFloat(matrix[row * C + col]);
				}
				std::cout << "\n";
			}
			for (int col = 0; col < 6; col++) {
				std::cout << "  ....       ";
			}
			std::cout << "\n";
			for (int row = R - 10; row < R; row++) {
				for (int col = 0; col < 3; col++) {
					printFloat(matrix[row * C + col]);
				}
				std::cout << "  ....      ";
				for (int col = C - 2; col < C; col++) {
					printFloat(matrix[row * C + col]);
				}
				std::cout << "\n";
			}
		}
	}
}