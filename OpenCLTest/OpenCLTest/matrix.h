#ifndef MATRIX_H
#define MATRIX_H

#include "gpu.h"

#include <iostream>
#include <cstdio>
#include <string>
#include <random>
#include <exception>

// Instantiation of kernels and such for matrix class
// Need global instance to prevent multiple template instances
struct MatrixKernelPackage {
public:
	Context context;
	cl_command_queue cmd_q;
	MatrixMultiplicationKernel mmk;

	MatrixKernelPackage();
};

void matrix_print(float * data, unsigned int R, unsigned int C);

// Dot-product of two matrices using a naive CPU algorithm
void matrix_naive_multiply(
	float * a, float * b, float * c,
	unsigned int R_left, unsigned int C_left, unsigned int C_right);

// Sets elements of matrix to random floats inside of [-1.0, 1.0]
void matrix_set_random(float * data, unsigned int n_elements);

void matrix_transpose(float * in, float * out, unsigned int rows, unsigned int cols);

class Matrix {
protected:
	float * data;

	unsigned int rows, cols;

	static MatrixKernelPackage matrix_kernel_package;

public:
	// Initialize an invalid matrix
	Matrix() {
		rows = cols = 0;
	}

	// Initialize a blank matrix with a default value of specified dimensions
	Matrix(unsigned int rows, unsigned int cols, float value = 0) {
		this->rows = rows, this->cols = cols;
		this->data = new float[rows * cols];
		for (int i = 0; i < rows * cols; i++) {
			data[i] = value;
		}
	}
	// Initialize a matrix from existing data with specified dimensions
	// Data must be row-major ({[0][0], [0][1], [0][2], ...})
	Matrix(unsigned int rows, unsigned int cols, float * data, size_t n_elements) {
		this->rows = rows, this->cols = cols;
		if (n_elements > rows * cols) throw 1;

		this->data = new float[rows * cols];
		memcpy(this->data, data, sizeof(float) * n_elements);
	}

	// Set elements of matrix to random floats inside of [-1.0, 1.0]
	inline void set_random() {
		matrix_set_random(data, rows * cols);
	}

	// Get the sum of the elements of the matrix
	float sum() const {
		float output = 0;
		for (unsigned int i = 0; i < rows * cols; i++) {
			output += data[i];
		}
		return output;
	}

	// Determine if the matrix is validly-initialized
	inline bool is_valid() const {
		return rows == 0 || cols == 0;
	}

protected:
	const static int ERROR_MATRIX_DIM = -10;
	// Check that the size of the matrix transposing into is correct
	inline static bool dim_check_trans(Matrix & left, Matrix & result) {
		return	left.rows == result.cols &&
				left.cols == result.rows;
	}
	// Check that the sizes of the matrices of the dot product are correct
	inline static bool dim_check_dot(Matrix & left, Matrix & right, Matrix & result) {
		return	left.rows == result.rows &&
				right.cols == result.cols &&
				left.cols == right.rows;
	}
	// Check that the sizes of the matrices in a normal matrix operation are correct
	inline static bool dim_check_mat(Matrix & left, Matrix & right, Matrix & result) {
		return	left.rows == right.rows &&
				right.rows == result.rows &&
				left.cols == right.cols &&
				right.cols == result.cols;
	}
	// Check that the sizes of the matrices in a scalar operation are correct
	inline static bool dim_check_scalar(Matrix & left, Matrix & result) {
		return	left.rows == result.rows &&
				left.cols == result.cols;
	}

public:

	void transpose(Matrix & output);
	Matrix transpose();

	void dot(Matrix & right, Matrix & output);
	Matrix dot(Matrix & right);

	void add(Matrix & right, Matrix & output);
	Matrix add(Matrix & right);

	void subtract(Matrix & right, Matrix & output);
	Matrix subtract(Matrix & right);

	void multiply(Matrix & right, Matrix & output);
	Matrix multiply(Matrix & right);

	void divide(Matrix & right, Matrix & output);
	Matrix divide(Matrix & right);

	void negate(Matrix & output);
	Matrix negate();

	void apply(Matrix & output, float(*function)(float));
	Matrix apply(float(*function)(float));

	// Print the matrix.
	// Prints only a couple first/last rows/cols for lasrge matrices.
	void print() {
		matrix_print(data, rows, cols);
	}
};

#endif