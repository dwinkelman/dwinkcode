#ifndef NEURAL_NETWORK_H
#define NEURAL_NETWORK_H

#include "matrix.h"

float SIGMOID(float x) {
	return 1.0f / (1.0f + exp(-x));
}
float SIGMOID_DDX(float x) {
	return exp(-x) / (1.0f + exp(-x)) / (1.0 + exp(-x));
}

// Single layer artificial neural network
class NeuralNetwork {
/******************************************************************************
 *
 *		Data Structures
 *		---------------
 * 
 * Input Layer:		x	[size of input (A)			x	size of data (n)]
 * Hidden Layer:	A2	[size of hidden layer (B)	x	size of data (n)]
 * Output Layer:	y	[size of output (C)			x	size of data (n)]
 *
 * Weights 1:	W1	[size of hidden layer (B)	x	size of input layer (A)]
 * Weights 2:	W2	[size of output layer (C)	x	size of hidden layer (B)]
 *
 *
 *		Forward Propogation
 *		-------------------
 *
 *			y = f(A2)
 *			Z2 = W2 * h1
 *			A2 = f(Z1)
 *			A1 = W1 * x
 *			y = f(W2 * f(W1 * x))
 *
 *			f(x) = 1 / (1 + e^-x)
 *
 *
 *		Back Propogation
 *		----------------
 *
 *			f'(x) = e^-x / (1 + e^-x)^2
**/
protected:
	unsigned int input_size, hidden_size, output_size;
	Matrix W1, W2;
	float(*activation_func)(float);
	float(*activation_func_ddx)(float);

public:
	NeuralNetwork(unsigned int input_size, unsigned int hidden_size, unsigned int output_size) {
		this->input_size = input_size;
		this->hidden_size = hidden_size;
		this->output_size = output_size;

		W1 = Matrix(hidden_size, input_size);
		W2 = Matrix(output_size, hidden_size);
		W1.set_random();
		W2.set_random();

		activation_func = SIGMOID;
		activation_func_ddx = SIGMOID_DDX;
	}

	Matrix forward(Matrix & input) {
		Matrix A1 = W1.dot(input);
		Matrix h1 = A1.apply(activation_func);
		Matrix A2 = W2.dot(h1);
		return A2.apply(activation_func);
	}
};

#endif