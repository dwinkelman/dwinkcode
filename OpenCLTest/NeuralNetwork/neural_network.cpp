#include "neural_network.h"

#ifdef DOUBLE_MATRIX
const float_ h = 1e-13;
#else
const float_ h = 2.5e-6;
#endif

/* 1 / (1 + e^-x) */
float_ sigmoid(float_ x) {
	return 1.0 / (1.0 + exp(-x));
}

/* e^-x / (1 + e^-x) ^ 2 */
float_ sigmoid_ddx(float_ x) {
	float negexp = exp(-x);
	return negexp / (1.0 + negexp) / (1.0 + negexp);
}

/* (e^-2x - e^-x) / (1 + e^-x) ^ 3 */
float_ sigmoid_d2dx2(float_ x) {
	float negexp = exp(-x);
	float plus1 = 1.0 + negexp;
	return (negexp * negexp - negexp) / plus1 / plus1 / plus1;
}

NeuralNetwork::NeuralNetwork(
	unsigned int input_size,
	unsigned int hidden_size,
	unsigned int output_size) {

	this->_input_size = input_size;
	this->_hidden_size = hidden_size;
	this->_output_size = output_size;

	m.W1 = Matrix(input_size, hidden_size);
	m.W1.set_random();
	m.W2 = Matrix(hidden_size, output_size);
	m.W2.set_random();

	set_function(sigmoid, sigmoid_ddx, sigmoid_d2dx2);

	/*
	mat_sizes[0]	= { &z2,			_N,				_hidden_size };
	mat_sizes[1]	= { &a2,			_N,				_hidden_size };
	mat_sizes[2]	= { &z3,			_N,				_output_size };
	mat_sizes[3]	= { &a3,			_N,				_output_size };
	mat_sizes[4]	= { &difference,	_N,				_output_size };
	mat_sizes[5]	= { &fprime_z3,		_N,				_output_size };
	mat_sizes[6]	= { &delta3,		_N,				_output_size };
	mat_sizes[7]	= { &a2_T,			_hidden_size,	_N };
	mat_sizes[8]	= { &W2_T,			_output_size,	_hidden_size };
	mat_sizes[9]	= { &delta3_dot,	_N,				_hidden_size };
	mat_sizes[10]	= { &fprime_z2,		_N,				_hidden_size };
	mat_sizes[11]	= { &delta2,		_N,				_hidden_size };
	mat_sizes[12]	= { &x_T,			_input_size,	_N };
	*/
}

void NeuralNetwork::set_function(
	float_(*activation_func)(float_),
	float_(*activation_func_ddx)(float_),
	float_(*activation_func_d2dx2)(float_)) {

	this->_activation_func = activation_func;
	this->_activation_func_ddx = activation_func_ddx;
	this->_activation_func_d2dx2 = activation_func_d2dx2;
}

void NeuralNetwork::_forward(InMatrix input) {

	// Validate parameter sizes
	if (!validate_input(input)) throw NN_ERROR_INVALID_INPUT;

	// Set variables to correct sizes if not already set
	// Optimization for repeated calls using same data
	/*
	int n = input.get_rows();
	if(z2.get_rows() != n || z2.get_cols() != _hidden_size) z2 = Matrix(n, _hidden_size);
	if(a2.get_rows() != n || a2.get_cols() != _hidden_size) a2 = Matrix(n, _hidden_size);
	if(z3.get_rows() != n || z3.get_cols() != _output_size) z3 = Matrix(n, _output_size);
	if(a3.get_rows() != n || a3.get_cols() != _output_size) a3 = Matrix(n, _output_size);
	*/
	/*
	for (int i = 0; i < 4; i++) {
		unsigned int r = (mat_sizes[i].rows == _N) ? input.get_rows() : mat_sizes[i].rows;
		unsigned int c = (mat_sizes[i].cols == _N) ? input.get_cols() : mat_sizes[i].cols;
		if (mat_sizes[i].matrix->get_rows() != r ||
			mat_sizes[i].matrix->get_cols() != c) {
			*mat_sizes[i].matrix = Matrix(r, c);
		}
	}
	*/

	input.dot(m.W1, m.z2);
	m.z2.apply(m.a2, _activation_func);
	m.a2.dot(m.W2, m.z3);
	m.z3.apply(m.a3, _activation_func);
}

float_ NeuralNetwork::_cost_func(InMatrix desired_output, InMatrix output) {

	// Validate that outputs are same size
	if (!desired_output.same_dims(output)) throw NN_ERROR_UNEQUAL_MATRIX_DIMS;

	m.cost.difference = output.subtract(desired_output);
	m.cost.square = m.cost.difference.multiply(m.cost.difference);
	float_ cost = 0.5 * m.cost.square.sum();
	return cost;
}

void NeuralNetwork::numeric_gradient(
	InMatrix input, InMatrix output, OutMatrix dJdW1, OutMatrix dJdW2) {

	// Validate parameter sizes
	if (!validate_input(input)) throw NN_ERROR_INVALID_INPUT;
	else if (!validate_output(output)) throw NN_ERROR_INVALID_OUTPUT;
	else if (!validate_input_output(input, output)) throw NN_ERROR_INVALID_INPUT_OUTPUT_PAIR;

	// Resize outputs
	dJdW1 = Matrix(_input_size, _hidden_size);
	dJdW2 = Matrix(_hidden_size, _output_size);

	// With respect to W1
	for (int i = 0; i < _input_size * _hidden_size; i++) {
		m.W1.set(i, m.W1.get(i) + h);
		_forward(input); // output is a3
		float_ higher = _cost_func(output, m.a3);

		m.W1.set(i, m.W1.get(i) - 2 * h);
		_forward(input);
		float_ lower = _cost_func(output, m.a3);

		dJdW1.set(i, (higher - lower) / (2 * h));

		// reset the weight
		m.W1.set(i, m.W1.get(i) + h);
	}
	m.grad.dJdW1 = dJdW1;

	// With respect to W2
	for (int i = 0; i < _hidden_size * _output_size; i++) {
		m.W2.set(i, m.W2.get(i) + h);
		_forward(input); // output is a3
		float_ higher = _cost_func(output, m.a3);

		m.W2.set(i, m.W2.get(i) - 2 * h);
		_forward(input);
		float_ lower = _cost_func(output, m.a3);

		dJdW2.set(i, (higher - lower) / (2 * h));

		// reset the weight
		m.W2.set(i, m.W2.get(i) + h);
	}
	m.grad.dJdW2 = dJdW2;
}

void NeuralNetwork::analytic_gradient(
	InMatrix input, InMatrix output, OutMatrix dJdW1, OutMatrix dJdW2) {

	// Validate parameter sizes
	if (!validate_input(input)) throw NN_ERROR_INVALID_INPUT;
	else if (!validate_output(output)) throw NN_ERROR_INVALID_OUTPUT;
	else if (!validate_input_output(input, output)) throw NN_ERROR_INVALID_INPUT_OUTPUT_PAIR;

	/*
	// Set variables to correct sizes if not already set
	// Optimization for repeated calls using same data
	for (int i = 4; i < 13; i++) {
		unsigned int r = (mat_sizes[i].rows == _N) ? input.get_rows() : mat_sizes[i].rows;
		unsigned int c = (mat_sizes[i].cols == _N) ? input.get_rows() : mat_sizes[i].cols;
		if (mat_sizes[i].matrix->get_rows() != r ||
			mat_sizes[i].matrix->get_cols() != c) {
			*mat_sizes[i].matrix = Matrix(r, c);
		}
	}
	if(dJdW1.get_rows() != _input_size || dJdW1.get_cols() != _hidden_size)
		dJdW1 = Matrix(_input_size, _hidden_size);
	if(dJdW2.get_rows() != _hidden_size || dJdW2.get_cols() != _output_size)
		dJdW2 = Matrix(_hidden_size, _output_size);
	*/

	// Generate values for activity at different layers
	_forward(input);

	// With respect to W2
	m.a3.subtract(output, m.bp.difference);					// y - y'
	m.z3.apply(m.bp.fprime_z3, _activation_func_ddx);		// f'(z3)
	m.bp.difference.multiply(m.bp.fprime_z3, m.bp.delta3);	// (y - y') f'(z3)
	m.a2.transpose(m.bp.a2_T);								// a2(T)
	m.bp.a2_T.dot(m.bp.delta3, dJdW2);						// a2(T) * [(y - y') f'(z3)]

	// With respect to W1
	m.W2.transpose(m.bp.W2_T);								// W2(T)
	m.bp.delta3.dot(m.bp.W2_T, m.bp.delta3_dot);			// [(y - y') f'(z3)] * W2(T)
	m.z2.apply(m.bp.fprime_z2, _activation_func_ddx);		// f'(z2)
	m.bp.delta3_dot.multiply(m.bp.fprime_z2, m.bp.delta2);	// [[(y - y') f'(z3)] * W2(T)] f'(z2)
	input.transpose(m.bp.x_T);								// x(T)
	m.bp.x_T.dot(m.bp.delta2, dJdW1);						// x(T) * [[[(y - y') f'(z3)] * W2(T)] f'(z2)]
}

float_ NeuralNetwork::gradient_descent(
	InMatrix input, InMatrix output, NeuralNetwork::GradientDescentParams params) {
	
	// Validate parameter sizes
	if (!validate_input(input)) throw NN_ERROR_INVALID_INPUT;
	else if (!validate_output(output)) throw NN_ERROR_INVALID_OUTPUT;
	else if (!validate_input_output(input, output)) throw NN_ERROR_INVALID_INPUT_OUTPUT_PAIR;

	float_ cost = params.cost_target + 1.0f;
	for (unsigned int i = 0;
		i < params.max_iterations && cost > params.cost_target;
		i++) {
		cost = _gradient_descent(input, output, params.step_size);
		//std::cout << "Cost (iteration " << i << "): " << cost << '\n';
	}

	return cost;
}

float_ NeuralNetwork::_gradient_descent(
	InMatrix input, InMatrix output, float_ step_size) {

	// Validate parameter sizes
	if (!validate_input(input)) throw NN_ERROR_INVALID_INPUT;
	else if (!validate_output(output)) throw NN_ERROR_INVALID_OUTPUT;
	else if (!validate_input_output(input, output)) throw NN_ERROR_INVALID_INPUT_OUTPUT_PAIR;

	/*
	// Set variables to correct sizes if not already set
	// Optimization for repeated calls using same data
	if(_dJdW1.get_rows() != _input_size || _dJdW1.get_cols() != _hidden_size)
		_dJdW1 = Matrix(_input_size, _hidden_size);
	if(_dJdW2.get_rows() != _hidden_size || _dJdW2.get_cols() != _output_size)
		_dJdW2 = Matrix(_hidden_size, _output_size);
	*/

	analytic_gradient(input, output, m.grad.dJdW1, m.grad.dJdW2);

	m.grad.dJdW1.self_multiply(step_size);
	m.grad.dJdW2.self_multiply(step_size);

	m.W1.subtract(m.grad.dJdW1, m.W1);
	m.W2.subtract(m.grad.dJdW2, m.W2);

	m.grad.dJdW1.self_multiply(1.0f / step_size);
	m.grad.dJdW2.self_multiply(1.0f / step_size);

	return _cost_func(output, m.a3);
}

float_ NeuralNetwork::_cost_func_gradient(InMatrix dJdW1, InMatrix dJdW2) {
	Matrix W1_sqr = dJdW1.multiply(dJdW1);
	Matrix W2_sqr = dJdW2.multiply(dJdW2);
	return 0.5f * (W1_sqr.sum() + W2_sqr.sum());
}

void NeuralNetwork::numeric_second_gradient(
	InMatrix input, InMatrix output, OutMatrix dLdW1, OutMatrix dLdW2) {

	// Validate parameter sizes
	if (!validate_input(input)) throw NN_ERROR_INVALID_INPUT;
	else if (!validate_output(output)) throw NN_ERROR_INVALID_OUTPUT;
	else if (!validate_input_output(input, output)) throw NN_ERROR_INVALID_INPUT_OUTPUT_PAIR;

	/*
	// Set variables to correct sizes if not already set
	// Optimization for repeated calls using same data
	if(_dJdW1.get_rows() != _input_size || _dJdW1.get_cols() != _hidden_size)
		_dJdW1 = Matrix(_input_size, _hidden_size);
	if(_dJdW2.get_rows() != _hidden_size || _dJdW2.get_cols() != _output_size)
		_dJdW2 = Matrix(_hidden_size, _output_size);
	*/

	dLdW1 = Matrix(_input_size, _hidden_size);
	dLdW2 = Matrix(_hidden_size, _output_size);

	// With respect to W1
	for (int i = 0; i < _input_size * _hidden_size; i++) {
		m.W1.set(i, m.W1.get(i) + h);
		analytic_gradient(input, output, m.grad.dJdW1, m.grad.dJdW2);
		float_ higher = _cost_func_gradient(m.grad.dJdW1, m.grad.dJdW2);

		m.W1.set(i, m.W1.get(i) - 2 * h);
		analytic_gradient(input, output, m.grad.dJdW1, m.grad.dJdW2);
		float_ lower = _cost_func_gradient(m.grad.dJdW1, m.grad.dJdW2);

		dLdW1.set(i, (higher - lower) / (2 * h));

		// reset the weight
		m.W1.set(i, m.W1.get(i) + h);
	}

	// With respect to W1
	for (int i = 0; i < _hidden_size * _output_size; i++) {
		m.W2.set(i, m.W2.get(i) + h);
		analytic_gradient(input, output, m.grad.dJdW1, m.grad.dJdW2);
		float_ higher = _cost_func_gradient(m.grad.dJdW1, m.grad.dJdW2);

		m.W2.set(i, m.W2.get(i) - 2 * h);
		analytic_gradient(input, output, m.grad.dJdW1, m.grad.dJdW2);
		float_ lower = _cost_func_gradient(m.grad.dJdW1, m.grad.dJdW2);

		dLdW2.set(i, (higher - lower) / (2 * h));

		// reset the weight
		m.W2.set(i, m.W2.get(i) + h);
	}
}

void NeuralNetwork::analytic_second_gradient(
	InMatrix input, InMatrix output, OutMatrix dLdW1, OutMatrix dLdW2) {

	// Validate parameter sizes
	if (!validate_input(input)) throw NN_ERROR_INVALID_INPUT;
	else if (!validate_output(output)) throw NN_ERROR_INVALID_OUTPUT;
	else if (!validate_input_output(input, output)) throw NN_ERROR_INVALID_INPUT_OUTPUT_PAIR;
	
	// Load back propagation, activity, and gradient variables
	analytic_gradient(input, output, m.grad.dJdW1, m.grad.dJdW2);

	Matrix a2_fprime_z3 = m.bp.a2_T.dot(m.bp.fprime_z3);
	Matrix dydW2 = m.bp.a2_T.dot(m.bp.fprime_z3);
	Matrix a2_T_sqr = m.bp.a2_T.multiply(m.bp.a2_T);
	Matrix difference = m.a3.subtract(output);
	Matrix f_double_prime_z3 = m.z3.apply(_activation_func_d2dx2);
	Matrix delta3 = difference.multiply(f_double_prime_z3);
	Matrix left = a2_fprime_z3.multiply(dydW2);
	Matrix right = a2_T_sqr.dot(delta3);
	Matrix sum = left.add(right);
	dLdW2 = m.grad.dJdW2.multiply(sum);
}