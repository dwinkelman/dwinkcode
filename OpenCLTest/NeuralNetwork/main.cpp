#include "neural_network.h"

void gradient_check() {
	NeuralNetwork nn(4, 6, 3);
	Matrix input_data(5, 4);
	input_data.set_random();
	Matrix output_data(5, 3);
	output_data.set_random(0.0f, 1.0f);

	std::cout << "Input Data:\n";
	input_data.print();
	std::cout << "Output Data:\n";
	output_data.print();

	std::cout << "Current Forward:\n";
	nn.forward(input_data).print();

	std::cout << "Cost Function:\n";
	std::cout << nn.cost(input_data, output_data) << '\n';

	// Use numeric gradient to test analytic gradient
	#if 1

	Matrix dJdW1_num, dJdW2_num;
	nn.numeric_gradient(input_data, output_data, dJdW1_num, dJdW2_num);

	Matrix dJdW1_ana, dJdW2_ana;
	nn.analytic_gradient(input_data, output_data, dJdW1_ana, dJdW2_ana);

	std::cout << "Numeric Gradient (W1):\n";
	dJdW1_num.print();
	std::cout << "Analytic Gradient (W1):\n";
	dJdW1_ana.print();

	std::cout << "Numeric Gradient (W2):\n";
	dJdW2_num.print();
	std::cout << "Analytic Gradient (W2):\n";
	dJdW2_ana.print();

	#endif
}

float_ gradient_descent(NeuralNetwork::GradientDescentParams params) {
	// Matrices with dimensions of multiples of 16
	NeuralNetwork nn(4, 8, 3);
	Matrix input_data(nn.n_params() * 8, 4);
	input_data.set_random(-1.0f, 1.0f);
	Matrix output_data(nn.n_params() * 8, 3);
	output_data.set_random(0.0f, 1.0f);

	float_ cost_initial = nn.cost(input_data, output_data);
	float_ cost_final = nn.gradient_descent(input_data, output_data, params);

	printf("Initial: %-16.4f Final: %-16.4f\n", cost_initial, cost_final);

	return cost_initial - cost_final;
}

void gradient_descent_test() {
	NeuralNetwork::GradientDescentParams params;
	params.cost_target = 0.01f;
	params.max_iterations = 100;

	float_ step_sizes[5]{ 0.01f, 0.02f, 0.03f, 0.04f, 0.05f };

	for (int i = 0; i < 5; i++) {
		std::cout << "Step Size: " << step_sizes[i] << '\n';
		params.step_size = step_sizes[i];

		for (int j = 0; j < 10; j++) {
			gradient_descent(params);
		}

		std::cout << '\n';
	}
}

int main() {

	NeuralNetwork nn(4, 6, 3);
	Matrix input_data(5, 4);
	input_data.set_random(-1.0f, 1.0f);
	Matrix output_data(5, 3);
	output_data.set_random(0.0f, 1.0f);

	Matrix dLdW1_num, dLdW2_num;
	nn.numeric_second_gradient(input_data, output_data, dLdW1_num, dLdW2_num);
	
	Matrix dLdW1_ana, dLdW2_ana;
	nn.analytic_second_gradient(input_data, output_data, dLdW1_ana, dLdW2_ana);

	std::cout << "Numeric with respect to W1:\n";
	dLdW1_num.print();
	std::cout << "Numeric with respect to W2:\n";
	dLdW2_num.print();
	std::cout << "Analytic with respect to W2:\n";
	dLdW2_ana.print();

	return 0;
}