#ifndef MATRIX_H
#define MATRIX_H

#define DOUBLE_MATRIX

#ifdef DOUBLE_MATRIX
typedef double float_;
#else
typedef float float_;
#endif

#ifndef DOUBLE_MATRIX
#include "gpu.h"
#endif

#include <iostream>
#include <cstdio>
#include <string>
#include <random>
#include <exception>

#ifndef DOUBLE_MATRIX
// Instantiation of kernels and such for matrix class
// Need global instance to prevent multiple template instances
struct MatrixKernelPackage {
public:
	Context context;
	cl_command_queue cmd_q;
	MatrixMultiplicationKernel mmk;

	MatrixKernelPackage();
};
#endif

void matrix_print(float_ * data, unsigned int R, unsigned int C);

// Dot-product of two matrices using a naive CPU algorithm
void matrix_naive_multiply(
	float_ * a, float_ * b, float_ * c,
	const unsigned int R_left, const unsigned int C_left, const unsigned int C_right);

// Sets elements of matrix to random float_s inside of [-1.0, 1.0]
void matrix_set_random(float_ * data, const unsigned int n_elements, const float_ lower_limit, const float_ upper_limit);

void matrix_transpose(const float_ * in, float_ * out, const unsigned int rows, const unsigned int cols);

class Matrix {
protected:
	float_ * data;

	unsigned int rows, cols;

	#ifndef DOUBLE_MATRIX
	static MatrixKernelPackage matrix_kernel_package;
	#endif

public:
	// Initialize an invalid matrix
	Matrix() {
		rows = cols = 0;
		data = NULL;
	}

	// Initialize a blank matrix with a default value of specified dimensions
	Matrix(const unsigned int rows, const unsigned int cols, const float_ value = 0) {
		this->rows = rows, this->cols = cols;
		this->data = new float_[rows * cols];
		memset(data, value, rows * cols);
	}
	// Initialize a matrix from existing data with specified dimensions
	// Data must be row-major ({[0][0], [0][1], [0][2], ...})
	Matrix(const unsigned int rows, const unsigned int cols, const float_ * data, const size_t n_elements) {
		this->rows = rows, this->cols = cols;
		if (n_elements > rows * cols) throw 1;

		this->data = new float_[rows * cols];
		memcpy(this->data, data, sizeof(float_) * n_elements);
	}
	// Copy constructor
	Matrix(const Matrix & other) {
		this->rows = other.rows, this->cols = other.cols;
			
		this->data = new float_[rows * cols];
		memcpy(this->data, other.data, rows * cols * sizeof(float_));
	}
	// Copy assignment
	Matrix & operator =(const Matrix & other) {
		// if size of data is same, no need to delete and reallocate
		if (this->rows * this->cols != other.rows * other.cols) {
			this->rows = other.rows, this->cols = other.cols;
			delete[] this->data;
			this->data = new float_[rows * cols];
		}
		memcpy(this->data, other.data, rows * cols * sizeof(float_));

		return *this;
	}
	~Matrix() {
		delete[] data;
	}

	// Changes the size of the matrix and sets to zero if the arguments passed
	// are different than the current state; otherwise nothing happens.
	void resize(unsigned int rows, unsigned int cols) {
		if (rows != this->rows || cols != this->cols) {
			this->rows = rows, this->cols = cols;

			delete[] data;
			data = new float_[rows * cols];
		}
	}

	// Set elements of matrix to random float_s inside of a range
	// [-1.0, 1.0] is default
	inline void set_random(const float_ lower_limit = -1.0f, const float_ upper_limit = 1.0f) {
		matrix_set_random(data, rows * cols, lower_limit, upper_limit);
	}

	// Get the sum of the elements of the matrix
	float_ sum() const {
		float_ output = 0;
		for (unsigned int i = 0; i < rows * cols; i++) {
			output += data[i];
		}
		return output;
	}

	// Determine if the matrix is validly-initialized
	inline bool is_valid() const {
		return rows == 0 || cols == 0;
	}

	inline void set(const unsigned int index, const float_ value) {
		data[index] = value;
	}
	inline void set(const unsigned int row, const unsigned int col, const float_ value) {
		data[row * cols + col] = value;
	}
	inline float_ get(const unsigned int index) const {
		return data[index];
	}
	inline float_ get(const unsigned int row, const unsigned int col) const {
		return data[row * cols + col];
	}

	inline unsigned int get_rows() const {
		return rows;
	}
	inline unsigned int get_cols() const {
		return cols;
	}

	inline bool same_dims(const Matrix & other) const {
		return dim_check_scalar(*this, other);
	}

protected:
	const static int ERROR_MATRIX_DIM = -10;
	// Check that the size of the matrix transposing into is correct
	inline static bool dim_check_trans(const Matrix & left, const Matrix & result) {
		return	left.rows == result.cols &&
				left.cols == result.rows;
	}
	// Check that the sizes of the matrices of the dot product are correct
	inline static bool dim_check_dot(const Matrix & left, const Matrix & right, const Matrix & result) {
		return	left.rows == result.rows &&
				right.cols == result.cols &&
				left.cols == right.rows;
	}
	// Check that the sizes of the matrices in a normal matrix operation are correct
	inline static bool dim_check_mat(const Matrix & left, const Matrix & right, const Matrix & result) {
		return	left.rows == right.rows &&
				right.rows == result.rows &&
				left.cols == right.cols &&
				right.cols == result.cols;
	}
	// Check that the sizes of the matrices in a scalar operation are correct
	inline static bool dim_check_scalar(const Matrix & left, const Matrix & result) {
		return	left.rows == result.rows &&
				left.cols == result.cols;
	}

public:

	void transpose(Matrix & output) const;
	Matrix transpose() ;

	void dot(const Matrix & right, Matrix & output) const;
	Matrix dot(const Matrix & right) const;

	void add(const Matrix & right, Matrix & output) const;
	Matrix add(const Matrix & right) const;

	void subtract(const Matrix & right, Matrix & output) const;
	Matrix subtract(const Matrix & right) const;

	void multiply(const Matrix & right, Matrix & output) const;
	Matrix multiply(const Matrix & right) const;
	void self_multiply(const float_ x);

	void divide(const Matrix & right, Matrix & output) const;
	Matrix divide(const Matrix & right) const;

	void negate(Matrix & output) const;
	Matrix negate() const;

	void apply(Matrix & output, float_(*function)(float_)) const;
	Matrix apply(float_(*function)(float_)) const;

	// Print the matrix.
	// Prints only a couple first/last rows/cols for large matrices.
	void print() {
		matrix_print(data, rows, cols);
	}
};

#endif