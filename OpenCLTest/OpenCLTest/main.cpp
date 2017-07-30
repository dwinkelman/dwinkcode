#include "neural_network.h"

#define PTR(arr) &arr[0]

void test_matrices() {
	// Change the stack reserve if necessary ;)
	// Configuration Properties -> Linker -> System - > Stack Reserve
	// [96 x 96] is break-even point between CPU and GPU
	const unsigned int left_R = 4, left_C = 6, right_C = 4;
	Matrix a(left_R, left_C);
	Matrix b(left_C, right_C);
	a.set_random();
	b.set_random();

	Matrix c = a.transpose();
	a.print();
	b.print();
	c.print();
}


int main() {
	test_matrices();

	NeuralNetwork nn = NeuralNetwork(3, 5, 1);
	float_ data[] = { 0.4, 0.5, 0.6 };
	Matrix result = nn.forward(Matrix(3, 1, PTR(data), 3));
	result.print();

	return 0;
}