#include "matrix.h"
#include "gpu.h"

MatrixKernelPackage::MatrixKernelPackage() {
	// Error code output
	cl_int err;

	context = Context();
	cmd_q = clCreateCommandQueue(context.get_context(), context.get_devices()[0], NULL, &err);
	check_err(err, "clCreateCommandQueue");

	mmk = MatrixMultiplicationKernel(&context);
}
MatrixKernelPackage Matrix::matrix_kernel_package;

void MatrixMultiplicationKernel::multiply(cl_command_queue cmd_q,
	float * a, float * b, float * c,
	unsigned int left_R, unsigned int left_C, unsigned int right_C) {
	// Error code output
	cl_int err;

	// Allocate kernel memory
	cl_mem a_mem, b_mem, c_mem;
	a_mem = clCreateBuffer(
		context->get_context(),
		CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
		sizeof(cl_float) * left_R * left_C, a,
		&err);
	check_err(err, "clCreateBuffer A in MatrixMultiplier");
	b_mem = clCreateBuffer(
		context->get_context(),
		CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
		sizeof(cl_float) * left_C * right_C, b,
		&err);
	check_err(err, "clCreateBuffer B in MatrixMultiplier");
	c_mem = clCreateBuffer(
		context->get_context(),
		CL_MEM_READ_WRITE,
		sizeof(cl_float) * left_R * right_C, NULL, &err);
	check_err(err, "clCreateBuffer C in MatrixMultiplier");

	// Copy data into buffer
	err = clEnqueueWriteBuffer(
		cmd_q, a_mem, CL_TRUE,
		0, sizeof(cl_float) * left_R * left_C, a,
		NULL, NULL, NULL);
	check_err(err, "clEnqueueWriteBuffer A in MatrixMultiplier");
	err = clEnqueueWriteBuffer(
		cmd_q, b_mem, CL_TRUE,
		0, sizeof(cl_float) * left_C * right_C, b,
		NULL, NULL, NULL);
	check_err(err, "clEnqueueWriteBuffer B in MatrixMultiplier");

	unsigned int a_cols = left_C, b_cols = right_C;

	// Set kernel args
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &a_mem);
	check_err(err, "setKernelArg A in MatrixMultiplier");
	err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &b_mem);
	check_err(err, "setKernelArg B in MatrixMultiplier");
	err = clSetKernelArg(kernel, 2, sizeof(cl_mem), &c_mem);
	check_err(err, "setKernelArg C in MatrixMultiplier");
	err = clSetKernelArg(kernel, 3, sizeof(unsigned int), &a_cols);
	check_err(err, "setKernelArg wA in MatrixMultiplier");
	err = clSetKernelArg(kernel, 4, sizeof(unsigned int), &b_cols);
	check_err(err, "setKernelArg wB in MatrixMultiplier");

	// Launch kernel
	size_t local_work_size[2] = { 16, 16 }, global_work_size[2] = { right_C, left_R };
	err = clEnqueueNDRangeKernel(cmd_q, kernel, 2, NULL, global_work_size, local_work_size, 0, NULL, NULL);
	check_err(err, "clEnqueueNDRangeKernel");

	// Copy back results
	err = clEnqueueReadBuffer(cmd_q, c_mem, CL_TRUE, 0, sizeof(cl_float) * left_R * right_C, (void *)c, 0, NULL, NULL);
	check_err(err, "clEnqueueReadBuffer");
}

void matrix_set_random(float * data, unsigned int n_elements) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dis(-1.0, 1.0);
	for (unsigned int i = 0; i < n_elements; i++) {
		data[i] = dis(gen);
	}
}

void matrix_transpose(float * in, float * out, unsigned int rows, unsigned int cols) {
	for (unsigned int row = 0; row < rows; row++) {
		for (unsigned int col = 0; col < cols; col++) {
			out[col * cols + row] = in[row * rows + col];
		}
	}
}

void matrix_naive_multiply(
	float * a, float * b, float * c,
	unsigned int R_left, unsigned int C_left, unsigned int C_right) {

	for (unsigned int row = 0; row < R_left; row++) {
		for (unsigned int col = 0; col < C_right; col++) {
			float total = 0;
			for (unsigned int k = 0; k < C_left; k++) {
				total += a[row * C_left + k] * b[k * C_right + col];
			}
			c[row * C_right + col] = total;
		}
	}
}


static inline void printFloat(float x) {
	printf("%-16f", x);
}
static inline void printDots() {
	printf("   ....         ");
}
void matrix_print(float * data, unsigned int R, unsigned int C) {
	std::cout << "Matrix [" << R << " x " << C << "]\n";
	if (C <= 6) {
		if (R <= 20) {
			for (unsigned int row = 0; row < R; row++) {
				for (unsigned int col = 0; col < C; col++) {
					printFloat(data[row * C + col]);
				}
				std::cout << "\n";
			}
		}
		else {
			for (int row = 0; row < 10; row++) {
				for (unsigned int col = 0; col < C; col++) {
					printFloat(data[row * C + col]);
				}
				std::cout << "\n";
			}
			for (unsigned int col = 0; col < C; col++) {
				printDots();
			}
			std::cout << "\n";
			for (unsigned int row = R - 10; row < R; row++) {
				for (unsigned int col = 0; col < C; col++) {
					printFloat(data[row * C + col]);
				}
				std::cout << "\n";
			}
		}
	}
	else {
		if (R <= 20) {
			for (unsigned int row = 0; row < R; row++) {
				for (unsigned int col = 0; col < 3; col++) {
					printFloat(data[row * C + col]);
				}
				printDots();
				for (unsigned int col = C - 2; col < C; col++) {
					printFloat(data[row * C + col]);
				}
				std::cout << "\n";
			}
		}
		else {
			for (unsigned int row = 0; row < 10; row++) {
				for (unsigned int col = 0; col < 3; col++) {
					printFloat(data[row * C + col]);
				}
				printDots();
				for (unsigned int col = C - 2; col < C; col++) {
					printFloat(data[row * C + col]);
				}
				std::cout << "\n";
			}
			for (int col = 0; col < 6; col++) {
				printDots();
			}
			std::cout << "\n";
			for (unsigned int row = R - 10; row < R; row++) {
				for (unsigned int col = 0; col < 3; col++) {
					printFloat(data[row * C + col]);
				}
				printDots();
				for (unsigned int col = C - 2; col < C; col++) {
					printFloat(data[row * C + col]);
				}
				std::cout << "\n";
			}
		}
	}
}

void Matrix::transpose(Matrix & output) {
	// Check matrix dimensions
	if (!dim_check_trans(*this, output)) throw ERROR_MATRIX_DIM;

	for (unsigned int row = 0; row < rows; row++) {
		for (unsigned int col = 0; col < cols; col++) {
			output.data[col * output.cols + row] = data[row * cols + col];
		}
	}
}
Matrix Matrix::transpose() {
	Matrix output(cols, rows);
	this->transpose(output);
	return output;
}

void Matrix::dot(Matrix & right, Matrix & output) {
	// Check matrix dimensions
	if (!dim_check_dot(*this, right, output)) throw ERROR_MATRIX_DIM;

	// Choose the naive approach for small matrices, GPU for large
	if (rows * cols > 96 * 96 || right.rows * right.cols > 96 * 96) {
		matrix_kernel_package.mmk.multiply(
			matrix_kernel_package.cmd_q,
			this->data, right.data, output.data,
			rows, cols, right.cols);
	}
	else {
		matrix_naive_multiply(
			this->data, right.data, output.data,
			rows, cols, right.cols);
	}
}
Matrix Matrix::dot(Matrix & right) {
	Matrix output(rows, right.cols);

	// Check matrix dimensions
	if (!dim_check_dot(*this, right, output)) throw ERROR_MATRIX_DIM;

	this->dot(right, output);
	return output;
}

void Matrix::add(Matrix & right, Matrix & output) {
	// Check matrix dimensions
	if (!dim_check_mat(*this, right, output)) throw ERROR_MATRIX_DIM;

	for (int i = 0; i < rows * cols; i++) {
		output.data[i] = this->data[i] + right.data[i];
	}
}
Matrix Matrix::add(Matrix & right) {
	Matrix output(rows, cols);

	// Check matrix dimensions
	if (!dim_check_mat(*this, right, output)) throw ERROR_MATRIX_DIM;

	this->add(right, output);
	return output;
}

void Matrix::subtract(Matrix & right, Matrix & output) {
	// Check matrix dimensions
	if (!dim_check_mat(*this, right, output)) throw ERROR_MATRIX_DIM;

	for (int i = 0; i < rows * cols; i++) {
		output.data[i] = this->data[i] - right.data[i];
	}
}
Matrix Matrix::subtract(Matrix & right) {
	Matrix output(rows, cols);

	// Check matrix dimensions
	if (!dim_check_mat(*this, right, output)) throw ERROR_MATRIX_DIM;

	this->subtract(right, output);
	return output;
}

void Matrix::multiply(Matrix & right, Matrix & output) {
	// Check matrix dimensions
	if (!dim_check_mat(*this, right, output)) throw ERROR_MATRIX_DIM;

	for (int i = 0; i < rows * cols; i++) {
		output.data[i] = this->data[i] * right.data[i];
	}
}
Matrix Matrix::multiply(Matrix & right) {
	Matrix output(rows, cols);

	// Check matrix dimensions
	if (!dim_check_mat(*this, right, output)) throw ERROR_MATRIX_DIM;

	this->multiply(right, output);
	return output;
}

void Matrix::divide(Matrix & right, Matrix & output) {
	// Check matrix dimensions
	if (!dim_check_mat(*this, right, output)) throw ERROR_MATRIX_DIM;

	for (int i = 0; i < rows * cols; i++) {
		output.data[i] = this->data[i] / right.data[i];
	}
}
Matrix Matrix::divide(Matrix & right) {
	Matrix output(rows, cols);

	// Check matrix dimensions
	if (!dim_check_mat(*this, right, output)) throw ERROR_MATRIX_DIM;

	this->divide(right, output);
	return output;
}

void Matrix::negate(Matrix & output) {
	// Check matrix dimensions
	if (!dim_check_scalar(*this, output)) throw ERROR_MATRIX_DIM;

	for (int i = 0; i < rows * cols; i++) {
		output.data[i] = -this->data[i];
	}
}
Matrix Matrix::negate() {
	Matrix output(rows, cols);
	this->negate(output);
	return output;
}

void Matrix::apply(Matrix & output, float(*function)(float)) {
	// Check matrix dimensions
	if (!dim_check_scalar(*this, output)) throw ERROR_MATRIX_DIM;

	for (unsigned int i = 0; i < rows * cols; i++) {
		output.data[i] = function(data[i]);
	}
}
Matrix Matrix::apply(float(*function)(float)) {
	Matrix output(rows, cols);
	this->apply(output, function);
	return output;
}