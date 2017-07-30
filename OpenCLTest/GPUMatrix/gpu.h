#ifndef GPU_H
#define GPU_H

#include <utility>
#include <CL/cl.h>

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <iterator>

#define CALLBACK __stdcall

inline void check_err(cl_int err, const std::string name) {
	if (err != CL_SUCCESS) {
		std::cerr << "Error: " << name << " (" << err << ")\n";
		// exit(EXIT_FAILURE);
	}
}

inline void CALLBACK device_err_callback(const char * err_info, const void * private_info, size_t cb, void * user_data) {
	std::cout << "Error: Device: " << err_info << '\n';
}

class Context {
protected:
	cl_context context;
	cl_device_id devices[16];
	unsigned int n_devices;

public:
	Context();

	inline cl_context get_context() {
		return context;
	}
	inline unsigned int get_n_devices() {
		return n_devices;
	}
	inline cl_device_id * get_devices() {
		return devices;
	}
};

class Kernel {
protected:
	Context * context;
	cl_kernel kernel;

public:
	Kernel() {
		context = NULL;
		kernel = NULL;
	}

	Kernel(Context * context, const char * source, const std::string kernel_name, const char * compiler_options);
	Kernel(Context * context, const std::string path, const std::string kernel_name, const char * compiler_options);
};

extern const char * matrix_kernel;
class MatrixMultiplicationKernel : public Kernel {
public:
	MatrixMultiplicationKernel() { }
	MatrixMultiplicationKernel(Context * context)
		: Kernel(context, (std::string)"matrix.cl", "matrix_multiply", "") { }

	void multiply(cl_command_queue cmd_q,
		float * a, float * b, float * c,
		unsigned int left_R, unsigned int left_C, unsigned int right_C);
};

class MatrixMultiplicationKernelDouble : public Kernel {
public:
	MatrixMultiplicationKernelDouble() { }
	MatrixMultiplicationKernelDouble(Context * context)
		: Kernel(context, (std::string)"matrix.cl", "matrix_multiply_double", "") { }

	void multiply(cl_command_queue cmd_q,
		double * a, double * b, double * c,
		unsigned int left_R, unsigned int left_C, unsigned int right_C);
};

#endif