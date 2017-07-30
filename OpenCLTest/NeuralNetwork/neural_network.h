#ifndef NEURAL_NETWORK_H
#define NEURAL_NETWORK_H

#include <matrix.h>

/******************************************************************************
* Error codes
*/
#define NN_ERROR_INVALID_INPUT -1
#define NN_ERROR_INVALID_OUTPUT -2
#define NN_ERROR_UNEQUAL_MATRIX_DIMS -3
#define NN_ERROR_INVALID_INPUT_OUTPUT_PAIR -4
#define NN_ERROR_INVALID_W1 -5
#define NN_ERROR_INVALID_W2 -6


float_ sigmoid(float_ x);
float_ sigmoid_ddx(float_ x);
float_ sigmoid_d2dx2(float_ x);

typedef const Matrix & InMatrix;
typedef Matrix & OutMatrix;

class NeuralNetwork {
/******************************************************************************
*
*		Data Structures
*		---------------
*			Size of data:			n
*			Size of input layer:	A
*			Size of hidden layer:	B
*			Size of output layer:	C
*
*			Input Layer:		x	[n x A]
*			Hidden Layer:		A2	[n x B]
*			Output Layer:		y	[n x C]
*
*			Weights 1:			W1	[A x B]
*			Weights 2:			W2	[B x C]
*
*			Multiplication 2:	z2	[n x B]
*			Multiplication 3:	z3	[n x C]
*
*			Activation 2:		a2	[n x B]
*			Activation 3:		a3	[n x C]
*
*
*		Forward Propagation
*		-------------------
*			y:[n x C]	=	a3:[n x C]
*			a3:[n x C]	= f(z3:[n x C])
*			z3:[n x C]	=	a2:[n x B] * W2:[B x C]
*			a2:[n x B]	= f(z2:[n x B])
*			z2:[n x B]	=	 x:[n x A] * W1:[A x B]
*
*			y:[n x C]	= f( f( x:[n x A] * W1:[A x B] ) * W2:[B x C])
*
*			f(x) = 1 / (1 + e^-x)
*
*
*		Back Propagation
*		----------------
*			Cost Function:	J
*			Actual Output:	y	[n x C]
*			Desired Output:	y'	[n x C]
*
*			Gradient W1:	dJ/dW1	[A x B]
*			Gradient W2:	dJ/dW2	[B x C]
*
*			J = 1/2 sum( (y - y')^2 )
*			dJ/dy = y - y'
*
*			dJ/dW2 = (y - y') dy/dz3 dz3/dW2
*			                  f'(z3)   a2
*			[B x C]  [n x C] [n x C] [n x B]
*
*			dJ/dW2 = a2(T) * [(y - y') f'(z3)]
*			[B x C] [B x n]       [n x C]
*
*			dJ/dW1 = (y - y') dy/dz3 dz3/da2 da2/dz2 dz2/dW1
*					 --dJ/dW2 uses--   W2    f'(z2)     x
*			[A x B]      [n x C]     [B x C] [n x B] [n x A]
*
*			dJ/dW1 = x(T) * [[[(y - y') f'(z3)] * W2(T)] f'(z2)]
*			[A x B] [A x n]    ----[n x C]----   [C x B] [n x B]
*			                  ---------[n x B]---------
*			                 --------------[n x B]-------------
*
*			f'(x) = e^-x / (1 + e^-x)^2
*
*
*		Gradient Descent
*		----------------
*			Quasi-Newton's method to systematically minimize the cost function
*
*			W1(n+1) = W1(n) - k * dJ/dW1(n)
*			W2(n+1) = W2(n) - k * dJ/dW2(n)
*
*			k is a small number approximating (dJ/dW1) / (d2J/dW1)
*			0.01 < k < 0.04 seems to work well
*
*
*		Newton's Method
*		---------------
*			Gradient Cost Function:		L
*			
*			Second Gradient:
*				(analogous to Hessian, but same dimensions as W1or W2)
*								dL/dW1	[A x B]
*								dL/dW2	[B x C]
*
*			L1 = 1/2 sum( dJ/dW1 ^ 2 )
*			L2 = 1/2 sum( dJ/dW2 ^ 2 )
*			L = L1 + L2
*			
*			dL2/dW2 = dJ/dW2
**/
protected:
	unsigned int _input_size, _hidden_size, _output_size;

	struct Matrices {
		Matrix W1, W2;
		Matrix z2, a2, z3, a3;
		struct Gradients {
			Matrix dJdW1, dJdW2;
		} grad;
		struct BackpropVars {
			Matrix difference, fprime_z3, delta3, a2_T;
			Matrix W2_T, delta3_dot, fprime_z2, delta2, x_T;
		} bp;
		struct CostVars {
			Matrix difference, square;
		} cost;
	} m;

	/*
	// Weight parameters
	Matrix W1, W2;

	// Internal activities
	// These are updated every time _forward() is called
	Matrix z2, a2, z3, a3;

	// Variables for calculating dJdW2
	// These are updated every time analytic_gradient() is called
	Matrix difference, fprime_z3, delta3, a2_T;
	// Variables for calculating dJdW1
	// These are updated every time analytic_gradient() is called
	Matrix W2_T, delta3_dot, fprime_z2, delta2, x_T;

	// Storage for current gradients
	Matrix _dJdW1, _dJdW2;
	*/

	/*
	static const unsigned int _N = -1;
	struct MatrixSizes {
		Matrix * matrix;
		unsigned int rows;
		unsigned int cols;
	} mat_sizes[13];
	*/

	// Activation function
	float_(*_activation_func)(float_);
	// Derivative of the activation function
	float_(*_activation_func_ddx)(float_);
	// 2nd derivative of the activation function
	float_(*_activation_func_d2dx2)(float_);

public:
	NeuralNetwork(
		const unsigned int input_size,
		const unsigned int hidden_size,
		const unsigned int output_size);
	
	/* Set the weights to user-defined values. Constructor is random. */
	void set_weights(InMatrix W1, InMatrix W2);
	/* Set the activation function (and the derivatives of the function) */
	void set_function(float_(*activation_func)(float_), float_(*activation_func_ddx)(float_), float_(*activation_func_d2dx2)(float_));
	/* Get the number of parameters within the function. */
	inline unsigned int n_params() const {
		return m.W1.get_cols() * m.W1.get_rows() + m.W2.get_cols() * m.W2.get_rows();
	}

	/* Run an input through the neural network. */
	Matrix forward(InMatrix input) {
		_forward(input); // outputs to a3
		return m.a3;
	}

	/* Get the current value of the cost function. */
	float_ cost(InMatrix input, InMatrix output) {
		return _cost_func(output, forward(input));
	}

	/*
	Estimate the gradients of the neural network numerically. The gradient is
	how each weight affects the accuracy of the function. The user needs to 
	provide an input set and the desired output. Uses the secant method to
	approximate derivatives (df/dx ~= [f(x + h) - f(x - h)] / 2h).
	
	IMPORTANT:
	Should be used for testing ONLY as it is computationally inefficient.
	*/
	void numeric_gradient(InMatrix input, InMatrix output, OutMatrix dJdW1, OutMatrix dJdW2);

	/*
	Determine the exact gradients of the neural network analytically. The
	gradient is how each weight affects the accuracy of the function. The user
	needs to provide an input set and the desired output.
	*/
	void analytic_gradient(InMatrix input, InMatrix output, OutMatrix dJdW1, OutMatrix dJdW2);

	struct GradientDescentParams {
		float_ cost_target = 0.0f;
		unsigned int max_iterations = 100;
		float_ step_size = 0.05f;
	};
	/*
	Use simple gradient descent to converge towards a locally optimal parameter
	set.
	*/
	float_ gradient_descent(InMatrix input, InMatrix output, GradientDescentParams params);

	/*
	Estimate the gradient of the gradient cost function. This is used to find the
	step size for gradient descent.

	IMPORTANT:
	Should be used for testing ONLY as it is computationally inefficient.
	*/
	void numeric_second_gradient(InMatrix input, InMatrix output, OutMatrix dLdW1, OutMatrix dLdW2);

	/* Determine the exact gradient of the gradient cost function. */
	void analytic_second_gradient(InMatrix input, InMatrix output, OutMatrix dLdW1, OutMatrix dLdW2);

protected:
	/*
	Requires that z2, a2, z3, and a3 be initialized to proper size before
	calling this function. Output is a3. Intended for more efficient memory
	use and higher performance.
	*/
	void _forward(InMatrix input);

	/* Go down the gradient once. Returns the cost of the function before. */
	float_ _gradient_descent(InMatrix input, InMatrix output, float_ step_size);

	/*
	Cost function: how well the function derives the desired output from
	a given input. Lower is a better fit.
	*/
	float_ _cost_func(InMatrix desired_output, InMatrix output);

	/* How flat the gradient is. Needed for second-gradient calculation */
	float_ _cost_func_gradient(InMatrix dJdW1, InMatrix dJdW2);

	/* Check that the size of an input matrix is appropriate */
	inline bool validate_input(InMatrix input) {
		return input.get_cols() == _input_size;
	}
	/* Check that the size of an output matrix is appropriate */
	inline bool validate_output(InMatrix output) {
		return output.get_cols() == _output_size;
	}
	/* Check that the input and output agree in number of samples */
	inline bool validate_input_output(InMatrix input, InMatrix output) {
		return input.get_rows() == output.get_rows();
	}
	/* Check that a W1 parameter is the correct dimension */
	inline bool validate_W1(InMatrix _W1) {
		return _W1.get_rows() == _input_size && _W1.get_cols() == _hidden_size;
	}
	/* Check that a W2 parameter is the correct dimension */
	inline bool validate_W2(InMatrix _W2) {
		return _W2.get_rows() == _hidden_size && _W2.get_cols() == _output_size;
	}
};


#endif